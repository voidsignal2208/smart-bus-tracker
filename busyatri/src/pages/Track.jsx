import React, { useState, useRef, useEffect, useCallback } from 'react'
import Navbar from '../components/Navbar'
import Footer from '../components/Footer'
import { useSearchParams } from 'react-router-dom'
import { GoogleMap, Marker, Polyline, useJsApiLoader } from '@react-google-maps/api'
import { Bus, Clock, Gauge, MapPin, Loader2, WifiOff, Star, X, Send, CheckCircle } from 'lucide-react'
import {
  getLatestLocation,
  getLocationHistory,
  getRouteStops,
  getRoutePolyline,
  submitFeedback,
  getBusFeedback,
  ApiError,
} from '../lib/api'
import { subscribeToBusLocation } from '../lib/trackingSocket'
import busIconUrl from '../assets/bus-icon.svg'

// Bundled into the frontend build and visible to anyone who inspects the
// page - that's expected for a Google Maps JS API key, as long as it's
// restricted (in Google Cloud Console) to your site's HTTP referrers.
// This is a DIFFERENT key from the backend's GOOGLE_MAPS_SERVER_KEY,
// which must stay server-side only.
const GOOGLE_MAPS_API_KEY = import.meta.env.VITE_GOOGLE_MAPS_API_KEY

// `geometry` gives us google.maps.geometry.encoding.decodePath, used below
// to turn the encoded polyline the backend forwards from Google Directions
// into an actual array of lat/lng points. Declared as a module-level
// constant (not inline in the component) because useJsApiLoader reloads
// the whole Maps script if it's ever passed a new array reference.
const MAP_LIBRARIES = ['geometry']

const containerStyle = {
  width: '100%',
  height: '100%',
}

// India's centroid - just an initial mount position before we know where
// anything actually is. fitBounds() (see below) takes over as soon as we
// have real stops/location data, so this is never what the user ends up
// looking at.
const DEFAULT_CENTER = { lat: 20.5937, lng: 78.9629 }

const SNAP_POINTS = [120, 400, 620]

const Track = () => {
  const [searchParams] = useSearchParams()
  const from = searchParams.get('from') || ''
  const to = searchParams.get('to') || ''
  const busNo = searchParams.get('busNo') || 'N/A'
  const busId = searchParams.get('busId')
  const routeId = searchParams.get('routeId')

  const { isLoaded } = useJsApiLoader({
    googleMapsApiKey: GOOGLE_MAPS_API_KEY,
    libraries: MAP_LIBRARIES,
  })

  const [height, setHeight] = useState(SNAP_POINTS[0])
  const startY = useRef(0)
  const startHeight = useRef(0)

  const [current, setCurrent] = useState(null) // { latitude, longitude, speed_kmh, timestamp }
  const [historyPath, setHistoryPath] = useState([])
  const [connectionState, setConnectionState] = useState('connecting') // connecting | live | error
  const [loadError, setLoadError] = useState('')

  // The route this bus is running: its stops (in sequence_order) and the
  // actual road-following path Google Directions draws through all of
  // them - not a straight line from stop to stop.
  const [routeStops, setRouteStops] = useState([])
  const [routePath, setRoutePath] = useState([]) // decoded {lat,lng}[] from the Directions polyline
  const [routeError, setRouteError] = useState('')

  // Passenger feedback & rating
  const [feedbackData, setFeedbackData] = useState({
    average_rating: 5,
    total_reviews: 0,
    avg_cleanliness: 5,
    crowded_pct: 0,
    ac_working_pct: 100,
    recent_comments: [],
  })
  const [showFeedbackModal, setShowFeedbackModal] = useState(false)
  const [submittingFeedback, setSubmittingFeedback] = useState(false)
  const [feedbackSubmitted, setFeedbackSubmitted] = useState(false)
  const [feedbackForm, setFeedbackForm] = useState({
    rating: 5,
    cleanliness: 5,
    ac_working: true,
    is_crowded: false,
    comment: '',
  })

  const mapRef = useRef(null)
  const didFitBoundsRef = useRef(false)

  const onMapLoad = useCallback((map) => {
    mapRef.current = map
  }, [])

  const fetchFeedback = useCallback(() => {
    if (!busId) return
    getBusFeedback(busId)
      .then((data) => {
        if (data) setFeedbackData(data)
      })
      .catch((err) => {
        console.warn('Could not fetch bus feedback:', err)
      })
  }, [busId])

  useEffect(() => {
    fetchFeedback()
  }, [fetchFeedback])

  const handleFeedbackSubmit = async (e) => {
    if (e) e.preventDefault()
    if (!busId) return
    setSubmittingFeedback(true)
    try {
      await submitFeedback(busId, feedbackForm)
      setFeedbackSubmitted(true)
      fetchFeedback()
      setTimeout(() => {
        setShowFeedbackModal(false)
        setFeedbackSubmitted(false)
        setFeedbackForm({
          rating: 5,
          cleanliness: 5,
          ac_working: true,
          is_crowded: false,
          comment: '',
        })
      }, 1400)
    } catch (err) {
      alert(err.message || 'Failed to submit feedback. Please try again.')
    } finally {
      setSubmittingFeedback(false)
    }
  }

  // Initial fetch: latest known location + recent history, so the map
  // isn't empty while the WebSocket connection is still opening.
  useEffect(() => {
    if (!busId) return
    let cancelled = false

    async function loadInitial() {
      try {
        const latest = await getLatestLocation(busId)
        if (!cancelled) setCurrent(latest)
      } catch (err) {
        // A 404 here just means this bus has no recorded position yet -
        // not a real error, the live feed below may still populate it.
        if (!cancelled && !(err instanceof ApiError && err.status === 404)) {
          setLoadError(err.message)
        }
      }

      try {
        const history = await getLocationHistory(busId, 50)
        if (!cancelled) {
          setHistoryPath(
            history
              .slice()
              .reverse()
              .map((h) => ({ lat: h.latitude, lng: h.longitude }))
          )
        }
      } catch {
        // History is a nice-to-have for the trail on the map; ignore failures.
      }
    }

    loadInitial()
    return () => {
      cancelled = true
    }
  }, [busId])

  // The route itself: stops, then the road-snapped path through them.
  // Only runs once the Maps JS API (and its geometry library) is loaded,
  // since decoding the polyline needs google.maps.geometry.encoding.
  useEffect(() => {
    if (!routeId || !isLoaded) return
    let cancelled = false

    async function loadRoute() {
      setRouteError('')
      try {
        const stops = await getRouteStops(routeId)
        if (cancelled) return
        const ordered = stops.slice().sort((a, b) => a.sequence_order - b.sequence_order)
        setRouteStops(ordered)

        if (ordered.length < 2) return // nothing to draw a path through

        const origin = ordered[0]
        const destination = ordered[ordered.length - 1]
        const waypoints = ordered.slice(1, -1).map((s) => ({ lat: s.latitude, lng: s.longitude }))

        const { polyline } = await getRoutePolyline({
          originLat: origin.latitude,
          originLng: origin.longitude,
          destLat: destination.latitude,
          destLng: destination.longitude,
          waypoints,
        })
        if (cancelled) return

        const decoded = window.google.maps.geometry.encoding
          .decodePath(polyline)
          .map((p) => ({ lat: p.lat(), lng: p.lng() }))
        setRoutePath(decoded)
      } catch (err) {
        if (!cancelled) {
          setRouteError(err instanceof ApiError ? err.message : "Failed to load this bus's route.")
        }
      }
    }

    loadRoute()
    return () => {
      cancelled = true
    }
  }, [routeId, isLoaded])

  // Live updates over /ws/tracking.
  useEffect(() => {
    if (!busId) return

    const unsubscribe = subscribeToBusLocation(busId, {
      onOpen: () => setConnectionState('live'),
      onLocation: (payload) => {
        setCurrent(payload)
        setHistoryPath((prev) => [...prev, { lat: payload.latitude, lng: payload.longitude }])
      },
      onError: () => setConnectionState('error'),
    })

    return unsubscribe
  }, [busId])

  // Fit the map to whatever we actually have (route stops + bus position)
  // exactly once, instead of recentering on every single GPS tick - that
  // was fighting the user any time they tried to pan or zoom around, and
  // is also why the map used to look "stuck" on one generic view.
  useEffect(() => {
    if (didFitBoundsRef.current) return
    if (!mapRef.current || !window.google) return
    if (routeStops.length === 0 && !current) return

    const bounds = new window.google.maps.LatLngBounds()
    routeStops.forEach((s) => bounds.extend({ lat: s.latitude, lng: s.longitude }))
    if (current) bounds.extend({ lat: current.latitude, lng: current.longitude })

    mapRef.current.fitBounds(bounds, 60)
    didFitBoundsRef.current = true
  }, [routeStops, current])

  const handlePointerDown = (e) => {
    e.target.setPointerCapture(e.pointerId)
    startY.current = e.clientY
    startHeight.current = height
  }

  const handlePointerMove = (e) => {
    if (e.buttons !== 1) return
    const delta = startY.current - e.clientY
    const newHeight = Math.min(Math.max(startHeight.current + delta, 80), 700)
    setHeight(newHeight)
  }

  const handlePointerUp = () => {
    const closest = SNAP_POINTS.reduce((a, b) =>
      Math.abs(b - height) < Math.abs(a - height) ? b : a
    )
    setHeight(closest)
  }

  if (!busId) {
    return (
      <div className='flex min-h-screen flex-col'>
        <Navbar />
        <div className='flex flex-1 items-center justify-center px-4 text-center text-gray-600'>
          No bus selected. Go back and pick a bus to track from the search results.
        </div>
        <Footer />
      </div>
    )
  }

  if (!GOOGLE_MAPS_API_KEY) {
    return (
      <div className='flex min-h-screen flex-col'>
        <Navbar />
        <div className='flex flex-1 items-center justify-center px-4 text-center text-gray-600'>
          Missing VITE_GOOGLE_MAPS_API_KEY - set it in busyatri/.env to load the map.
        </div>
        <Footer />
      </div>
    )
  }

  if (!isLoaded) return <div className='flex items-center justify-center bg-lime-100 h-screen'>Loading map...</div>

  const busIcon = {
    url: busIconUrl,
    scaledSize: new window.google.maps.Size(44, 44),
    anchor: new window.google.maps.Point(22, 22),
  }

  const stopIcon = {
    path: window.google.maps.SymbolPath.CIRCLE,
    scale: 6,
    fillColor: '#ffffff',
    fillOpacity: 1,
    strokeColor: '#1d4ed8',
    strokeWeight: 2,
  }

  return (
    <div className='flex min-h-screen flex-col'>
      <Navbar />
      <div className='relative h-[calc(100vh-110px)] w-full mt-16 overflow-hidden'>
        <GoogleMap
          mapContainerStyle={containerStyle}
          center={DEFAULT_CENTER}
          zoom={5}
          onLoad={onMapLoad}
        >
          {/* The actual route, snapped to roads through every stop - from
              the backend's Google Directions proxy, not a straight line
              between stops. */}
          {routePath.length > 1 && (
            <Polyline
              path={routePath}
              options={{ strokeColor: '#1d4ed8', strokeWeight: 5, strokeOpacity: 0.85 }}
            />
          )}

          {/* Where the bus has actually been, from recorded GPS pings -
              kept as its own thinner line so it reads as "breadcrumb
              trail" rather than "the route". */}
          {historyPath.length > 1 && (
            <Polyline
              path={historyPath}
              options={{ strokeColor: '#f97316', strokeWeight: 3, strokeOpacity: 0.9 }}
            />
          )}

          {routeStops.map((stop) => (
            <Marker
              key={stop.id}
              position={{ lat: stop.latitude, lng: stop.longitude }}
              icon={stopIcon}
              title={stop.name}
            />
          ))}

          {current && (
            <Marker
              position={{ lat: current.latitude, lng: current.longitude }}
              icon={busIcon}
              title={busNo}
              zIndex={999}
            />
          )}
        </GoogleMap>

        <div
          className='absolute bottom-0 left-0 right-0 bg-white rounded-t-2xl shadow-2xl transition-[height] duration-150 ease-out z-10'
          style={{ height: height }}
        >
          <div
            onPointerDown={handlePointerDown}
            onPointerMove={handlePointerMove}
            onPointerUp={handlePointerUp}
            className='w-full flex justify-center py-3 touch-none'
          >
            <div className='w-10 h-1.5 bg-gray-300 rounded-full' />
          </div>

          <div className='px-4 pb-4 overflow-y-auto space-y-4' style={{ height: height - 40 }}>
            <div className='border border-gray-200 rounded-xl p-4 space-y-3'>
              <div className='flex items-center justify-between'>
                <div className='flex items-center gap-2'>
                  <Bus size={18} className='text-stone-700' />
                  <span className='font-semibold text-base'>{busNo}</span>
                </div>
                <span className='flex items-center gap-1 text-xs font-medium'>
                  {connectionState === 'live' && <span className='h-2 w-2 rounded-full bg-lime-600' />}
                  {connectionState === 'connecting' && <Loader2 size={12} className='animate-spin' />}
                  {connectionState === 'error' && <WifiOff size={14} className='text-red-500' />}
                  {connectionState === 'live' ? 'Live' : connectionState === 'connecting' ? 'Connecting...' : 'Connection lost'}
                </span>
              </div>

              {(from || to) && (
                <div className='text-sm text-gray-600'>
                  {from || 'Origin'} <span className='mx-1'>⇄</span> {to || 'Destination'}
                </div>
              )}

              {!routeId && (
                <p className='text-xs text-amber-700 bg-amber-50 rounded-lg px-3 py-2'>
                  This bus wasn't opened from a matched route, so no route/stops are shown -
                  only its live position.
                </p>
              )}

              {routeError && <p className='text-xs text-red-600'>{routeError}</p>}

              {loadError && <p className='text-xs text-red-600'>{loadError}</p>}

              {!current && !loadError && (
                <p className='text-sm text-gray-500'>
                  No location has been recorded for this bus yet. It'll appear here as soon
                  as its driver starts sending GPS updates.
                </p>
              )}

              {current && (
                <>
                  <div className='flex items-center justify-between bg-stone-50 rounded-lg px-3 py-2'>
                    <div className='flex items-center gap-2 text-sm text-gray-700'>
                      <MapPin size={16} />
                      Position
                    </div>
                    <span className='text-sm font-medium'>
                      {current.latitude.toFixed(5)}, {current.longitude.toFixed(5)}
                    </span>
                  </div>

                  <div className='flex items-center justify-between bg-stone-50 rounded-lg px-3 py-2'>
                    <div className='flex items-center gap-2 text-sm text-gray-700'>
                      <Gauge size={16} />
                      Speed
                    </div>
                    <span className='text-sm font-medium'>
                      {current.speed_kmh != null ? `${Number(current.speed_kmh).toFixed(1)} km/h` : 'Unknown'}
                    </span>
                  </div>

                  <div className='flex items-center justify-between bg-stone-50 rounded-lg px-3 py-2'>
                    <div className='flex items-center gap-2 text-sm text-gray-700'>
                      <Clock size={16} />
                      Last update
                    </div>
                    <span className='text-sm font-medium'>
                      {current.timestamp ? new Date(current.timestamp).toLocaleTimeString() : 'Unknown'}
                    </span>
                  </div>
                </>
              )}

              {routeStops.length > 0 && (
                <div className='pt-2'>
                  <p className='mb-2 text-xs font-semibold text-gray-500'>STOPS ON THIS ROUTE</p>
                  <ol className='space-y-1'>
                    {routeStops.map((stop, i) => (
                      <li key={stop.id} className='flex items-center gap-2 text-sm text-gray-700'>
                        <span className='flex h-5 w-5 shrink-0 items-center justify-center rounded-full bg-blue-100 text-[10px] font-semibold text-blue-800'>
                          {i + 1}
                        </span>
                        {stop.name}
                      </li>
                    ))}
                  </ol>
                </div>
              )}

              {/* Passenger feedback & ratings */}
              <div className='border-t border-gray-100 pt-3 space-y-3'>
                <div className='flex items-center justify-between'>
                  <div className='flex items-center gap-1.5'>
                    <Star size={16} className='fill-amber-400 text-amber-400' />
                    <span className='text-sm font-semibold text-gray-800'>
                      {Number(feedbackData.average_rating).toFixed(1)}
                    </span>
                    <span className='text-xs text-gray-500'>
                      ({feedbackData.total_reviews} review{feedbackData.total_reviews === 1 ? '' : 's'})
                    </span>
                  </div>
                  <button
                    type='button'
                    onClick={() => setShowFeedbackModal(true)}
                    className='flex items-center gap-1.5 px-3 py-1.5 text-xs font-bold text-white bg-lime-700 hover:bg-lime-800 rounded-lg shadow-sm transition'
                  >
                    <Star size={13} />
                    Rate Your Journey
                  </button>
                </div>

                <div className='grid grid-cols-3 gap-2 text-center'>
                  <div className='bg-stone-50 rounded-lg py-2'>
                    <p className='text-xs font-semibold text-gray-800'>
                      {Number(feedbackData.avg_cleanliness).toFixed(1)}/5
                    </p>
                    <p className='text-[10px] text-gray-500'>Cleanliness</p>
                  </div>
                  <div className='bg-stone-50 rounded-lg py-2'>
                    <p className='text-xs font-semibold text-gray-800'>{feedbackData.ac_working_pct}%</p>
                    <p className='text-[10px] text-gray-500'>AC Working</p>
                  </div>
                  <div className='bg-stone-50 rounded-lg py-2'>
                    <p className='text-xs font-semibold text-gray-800'>{feedbackData.crowded_pct}%</p>
                    <p className='text-[10px] text-gray-500'>Crowded</p>
                  </div>
                </div>

                {feedbackData.recent_comments?.length > 0 && (
                  <div className='space-y-2'>
                    <p className='text-xs font-semibold text-gray-500'>RECENT FEEDBACK</p>
                    {feedbackData.recent_comments.map((c, i) => (
                      <div key={i} className='bg-stone-50 rounded-lg p-2.5 text-xs text-gray-700 space-y-1'>
                        <p>{c.comment}</p>
                        <div className='flex items-center gap-2 text-[10px] text-gray-500'>
                          <span>Cleanliness: {c.cleanliness}/5</span>
                          <span>•</span>
                          <span>AC: {c.ac_working ? 'Yes' : 'No'}</span>
                          <span>•</span>
                          <span>{c.is_crowded ? 'Crowded' : 'Not Crowded'}</span>
                        </div>
                      </div>
                    ))}
                  </div>
                )}
              </div>
            </div>
          </div>
        </div>
      </div>

      {/* Modal for feedback submission */}
      {showFeedbackModal && (
        <div className='fixed inset-0 z-50 flex items-center justify-center bg-black/50 backdrop-blur-xs p-4'>
          <div className='bg-white rounded-2xl max-w-md w-full p-6 shadow-2xl relative animate-in fade-in zoom-in-95 duration-200'>
            <button
              type='button'
              onClick={() => setShowFeedbackModal(false)}
              className='absolute top-4 right-4 text-gray-400 hover:text-gray-600 p-1 rounded-full hover:bg-gray-100 transition'
            >
              <X size={20} />
            </button>

            <h2 className='text-lg font-bold text-gray-900 mb-1'>Rate Your Journey</h2>
            <p className='text-xs text-gray-500 mb-4'>
              Your feedback updates live ratings and helps fellow passengers.
            </p>

            {feedbackSubmitted ? (
              <div className='py-8 text-center space-y-2'>
                <CheckCircle size={44} className='mx-auto text-emerald-600 animate-bounce' />
                <h3 className='text-base font-bold text-gray-800'>Thank you!</h3>
                <p className='text-xs text-gray-600'>Your feedback and rating have been recorded.</p>
              </div>
            ) : (
              <form onSubmit={handleFeedbackSubmit} className='space-y-4'>
                {/* Overall Star Rating */}
                <div>
                  <label className='block text-xs font-semibold text-gray-700 mb-1.5'>
                    Overall Experience Rating
                  </label>
                  <div className='flex items-center gap-2'>
                    {[1, 2, 3, 4, 5].map((star) => (
                      <button
                        key={star}
                        type='button'
                        onClick={() => setFeedbackForm((prev) => ({ ...prev, rating: star }))}
                        className='p-1 hover:scale-110 transition-transform'
                      >
                        <Star
                          size={28}
                          className={
                            star <= feedbackForm.rating
                              ? 'fill-amber-400 text-amber-400'
                              : 'text-gray-300 hover:text-amber-200'
                          }
                        />
                      </button>
                    ))}
                    <span className='ml-2 text-sm font-bold text-amber-600'>
                      {feedbackForm.rating} / 5
                    </span>
                  </div>
                </div>

                {/* Cleanliness Rating */}
                <div>
                  <label className='block text-xs font-semibold text-gray-700 mb-1.5'>
                    Bus Cleanliness Rating
                  </label>
                  <div className='flex gap-1.5'>
                    {[1, 2, 3, 4, 5].map((val) => (
                      <button
                        key={val}
                        type='button'
                        onClick={() => setFeedbackForm((prev) => ({ ...prev, cleanliness: val }))}
                        className={`flex-1 py-1.5 text-xs font-bold rounded-lg border transition ${
                          feedbackForm.cleanliness === val
                            ? 'bg-lime-700 text-white border-lime-700 shadow-xs'
                            : 'bg-stone-50 text-gray-700 border-gray-200 hover:bg-stone-100'
                        }`}
                      >
                        {val} ★
                      </button>
                    ))}
                  </div>
                </div>

                {/* AC and Crowdedness Toggles */}
                <div className='grid grid-cols-2 gap-3 pt-1'>
                  <div>
                    <label className='block text-xs font-semibold text-gray-700 mb-1.5'>
                      AC Working?
                    </label>
                    <div className='flex gap-1 bg-stone-100 p-1 rounded-lg'>
                      <button
                        type='button'
                        onClick={() => setFeedbackForm((prev) => ({ ...prev, ac_working: true }))}
                        className={`flex-1 py-1 text-xs font-medium rounded-md transition ${
                          feedbackForm.ac_working
                            ? 'bg-white text-lime-800 shadow-xs font-bold'
                            : 'text-gray-500'
                        }`}
                      >
                        Yes
                      </button>
                      <button
                        type='button'
                        onClick={() => setFeedbackForm((prev) => ({ ...prev, ac_working: false }))}
                        className={`flex-1 py-1 text-xs font-medium rounded-md transition ${
                          !feedbackForm.ac_working
                            ? 'bg-white text-rose-700 shadow-xs font-bold'
                            : 'text-gray-500'
                        }`}
                      >
                        No
                      </button>
                    </div>
                  </div>

                  <div>
                    <label className='block text-xs font-semibold text-gray-700 mb-1.5'>
                      Is Bus Crowded?
                    </label>
                    <div className='flex gap-1 bg-stone-100 p-1 rounded-lg'>
                      <button
                        type='button'
                        onClick={() => setFeedbackForm((prev) => ({ ...prev, is_crowded: true }))}
                        className={`flex-1 py-1 text-xs font-medium rounded-md transition ${
                          feedbackForm.is_crowded
                            ? 'bg-white text-orange-700 shadow-xs font-bold'
                            : 'text-gray-500'
                        }`}
                      >
                        Yes
                      </button>
                      <button
                        type='button'
                        onClick={() => setFeedbackForm((prev) => ({ ...prev, is_crowded: false }))}
                        className={`flex-1 py-1 text-xs font-medium rounded-md transition ${
                          !feedbackForm.is_crowded
                            ? 'bg-white text-lime-800 shadow-xs font-bold'
                            : 'text-gray-500'
                        }`}
                      >
                        No
                      </button>
                    </div>
                  </div>
                </div>

                {/* Comment / Review */}
                <div>
                  <label className='block text-xs font-semibold text-gray-700 mb-1'>
                    Comment or Remarks (Optional)
                  </label>
                  <textarea
                    rows={3}
                    value={feedbackForm.comment}
                    onChange={(e) => setFeedbackForm((prev) => ({ ...prev, comment: e.target.value }))}
                    placeholder='Share details about cleanliness, driver conduct, punctuality...'
                    className='w-full text-xs rounded-lg border border-gray-300 p-2.5 focus:border-lime-700 focus:outline-hidden focus:ring-1 focus:ring-lime-700'
                  />
                </div>

                {/* Action buttons */}
                <div className='flex items-center justify-end gap-2 pt-2'>
                  <button
                    type='button'
                    onClick={() => setShowFeedbackModal(false)}
                    className='px-4 py-2 text-xs font-semibold text-gray-600 hover:bg-gray-100 rounded-lg transition'
                  >
                    Cancel
                  </button>
                  <button
                    type='submit'
                    disabled={submittingFeedback}
                    className='flex items-center gap-1.5 px-5 py-2 text-xs font-bold text-white bg-lime-700 hover:bg-lime-800 disabled:opacity-50 rounded-lg shadow-sm transition'
                  >
                    <Send size={13} />
                    {submittingFeedback ? 'Submitting...' : 'Submit Feedback'}
                  </button>
                </div>
              </form>
            )}
          </div>
        </div>
      )}

      <Footer />
    </div>
  )
}

export default Track
