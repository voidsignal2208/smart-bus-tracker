import React, { useState, useRef, useEffect, useCallback } from 'react'
import Navbar from '../components/Navbar'
import Footer from '../components/Footer'
import { useSearchParams } from 'react-router-dom'
import { GoogleMap, Marker, DirectionsRenderer, Polyline, InfoWindow, useJsApiLoader } from '@react-google-maps/api'
import { Bus, Clock, AlertTriangle, MapPin, Gauge, Wifi, WifiOff, User, Phone, Star, Navigation, Sparkles, Wind, Users, CheckCircle, X, MessageSquare, ThumbsUp, Send } from 'lucide-react'
import { useBusTracking } from '../hooks/useBusTracking'
import { trackingService } from '../services/trackingService'
import { routeService } from '../services/routeService'
import { fleetService } from '../services/fleetService'

// Reads from .env (VITE_GOOGLE_MAPS_API_KEY)
const GOOGLE_MAPS_API_KEY = import.meta.env.VITE_GOOGLE_MAPS_API_KEY || ''

const containerStyle = { width: '100%', height: '100%' }
const SNAP_POINTS = [120, 400, 620]
const DEFAULT_CENTER = { lat: 28.6139, lng: 77.209 }

const staticInfo = {
  driver: 'Assigned Driver',
  contact: '+91 98765 43210',
  rating: 5,
}

const Track = () => {
  const [searchParams] = useSearchParams()
  const from = searchParams.get('from') || 'Origin'
  const to = searchParams.get('to') || 'Destination'
  const busId = searchParams.get('busId') || ''
  const routeId = searchParams.get('routeId') || ''

  const { isLoaded, loadError } = useJsApiLoader({
    googleMapsApiKey: GOOGLE_MAPS_API_KEY,
  })

  // WebSocket live tracking (works without login now)
  const { location: wsLocation, connected, error: wsError } = useBusTracking(busId)

  const [map, setMap] = useState(null)
  const [height, setHeight] = useState(SNAP_POINTS[0])
  const [directions, setDirections] = useState(null)
  const [busPosition, setBusPosition] = useState(null)
  const [busSpeed, setBusSpeed] = useState(0)
  const [lastUpdate, setLastUpdate] = useState(null)
  const [stops, setStops] = useState([])
  const [selectedMarker, setSelectedMarker] = useState(null)
  const startY = useRef(0)
  const startHeight = useRef(0)

  // Passenger Feedback & Rating state
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

  const onMapLoad = useCallback((mapInstance) => {
    setMap(mapInstance)
  }, [])

  const fetchFeedback = useCallback(() => {
    if (!busId) return
    fleetService
      .getBusFeedback(busId)
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
      await fleetService.submitFeedback(busId, feedbackForm)
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


  // Fetch initial location via REST
  useEffect(() => {
    if (!busId) return
    trackingService
      .getLatestLocation(busId)
      .then((data) => {
        if (data && data.latitude && data.longitude) {
          setBusPosition({ lat: parseFloat(data.latitude), lng: parseFloat(data.longitude) })
          setBusSpeed(data.speed_kmh ? parseFloat(data.speed_kmh) : 0)
          setLastUpdate(data.timestamp)
        }
      })
      .catch(() => {
        // No recorded location yet
      })
  }, [busId])

  // Fetch route stops from Supabase database
  useEffect(() => {
    if (!routeId) return
    routeService
      .getRouteStops(routeId)
      .then((data) => {
        if (Array.isArray(data) && data.length > 0) {
          const sortedStops = data.sort((a, b) => a.sequence_order - b.sequence_order)
          setStops(sortedStops)
        }
      })
      .catch(() => {})
  }, [routeId])

  // Update bus position from WebSocket real-time broadcast
  useEffect(() => {
    if (wsLocation) {
      setBusPosition({ lat: wsLocation.lat, lng: wsLocation.lng })
      setBusSpeed(wsLocation.speed || 0)
      setLastUpdate(wsLocation.timestamp)
    }
  }, [wsLocation])

  // Fit bounds to show all route stops and bus position
  useEffect(() => {
    if (!map || !window.google?.maps) return

    const bounds = new window.google.maps.LatLngBounds()
    let hasPoints = false

    if (busPosition) {
      bounds.extend(busPosition)
      hasPoints = true
    }

    stops.forEach((s) => {
      if (s.latitude && s.longitude) {
        bounds.extend({ lat: parseFloat(s.latitude), lng: parseFloat(s.longitude) })
        hasPoints = true
      }
    })

    if (hasPoints) {
      map.fitBounds(bounds, 60)
    }
  }, [map, stops, busPosition])

  // Build driving directions from real stop coordinates
  useEffect(() => {
    if (!isLoaded || !window.google?.maps || stops.length < 2) return

    const directionsService = new window.google.maps.DirectionsService()
    const origin = { lat: parseFloat(stops[0].latitude), lng: parseFloat(stops[0].longitude) }
    const destination = {
      lat: parseFloat(stops[stops.length - 1].latitude),
      lng: parseFloat(stops[stops.length - 1].longitude),
    }
    const waypoints = stops.slice(1, -1).map((s) => ({
      location: { lat: parseFloat(s.latitude), lng: parseFloat(s.longitude) },
      stopover: true,
    }))

    directionsService.route(
      { origin, destination, waypoints, travelMode: window.google.maps.TravelMode.DRIVING },
      (result, status) => {
        if (status === 'OK') {
          setDirections(result)
        } else {
          console.warn('Google DirectionsService status:', status)
        }
      }
    )
  }, [isLoaded, stops])

  // Bottom sheet drag handlers
  const handlePointerDown = (e) => {
    e.target.setPointerCapture(e.pointerId)
    startY.current = e.clientY
    startHeight.current = height
  }
  const handlePointerMove = (e) => {
    if (e.buttons !== 1) return
    const delta = startY.current - e.clientY
    setHeight(Math.min(Math.max(startHeight.current + delta, 80), 700))
  }
  const handlePointerUp = () => {
    const closest = SNAP_POINTS.reduce((a, b) =>
      Math.abs(b - height) < Math.abs(a - height) ? b : a
    )
    setHeight(closest)
  }

  const mapCenter = busPosition || (stops.length > 0
    ? { lat: parseFloat(stops[0].latitude), lng: parseFloat(stops[0].longitude) }
    : DEFAULT_CENTER)

  // Estimate ETA (rough: distance / speed)
  const estimateETA = () => {
    if (!busPosition || stops.length === 0 || busSpeed <= 0) return 'Calculating...'
    const dest = stops[stops.length - 1]
    const R = 6371
    const dLat = ((parseFloat(dest.latitude) - busPosition.lat) * Math.PI) / 180
    const dLon = ((parseFloat(dest.longitude) - busPosition.lng) * Math.PI) / 180
    const a =
      Math.sin(dLat / 2) ** 2 +
      Math.cos((busPosition.lat * Math.PI) / 180) *
        Math.cos((parseFloat(dest.latitude) * Math.PI) / 180) *
        Math.sin(dLon / 2) ** 2
    const dist = R * 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a))
    const hours = dist / busSpeed
    if (hours < 1) return `${Math.max(1, Math.round(hours * 60))} min`
    return `${Math.floor(hours)}h ${Math.round((hours % 1) * 60)}min`
  }

  const nextStop = stops.length > 0 ? stops[stops.length - 1].name : 'N/A'

  // Polyline coordinates fallback
  const polylineCoords = stops.map((s) => ({
    lat: parseFloat(s.latitude),
    lng: parseFloat(s.longitude),
  }))

  if (loadError) {
    return (
      <div className='flex min-h-screen flex-col'>
        <Navbar />
        <div className='flex flex-col items-center justify-center h-[calc(100vh-110px)] mt-16 p-6 text-center'>
          <AlertTriangle size={48} className='text-amber-600 mb-4' />
          <h2 className='text-xl font-bold text-gray-800 mb-2'>Google Maps API Error</h2>
          <p className='text-sm text-gray-600 max-w-md mb-4'>
            {loadError.message || 'Please ensure VITE_GOOGLE_MAPS_API_KEY is configured in busyatri/.env.'}
          </p>
        </div>
        <Footer />
      </div>
    )
  }

  if (!isLoaded) return <div className='flex items-center justify-center bg-lime-100 h-screen'>Loading map & route data...</div>

  return (
    <div className='flex min-h-screen flex-col'>
      <Navbar />
      <div className='relative h-[calc(100vh-110px)] w-full mt-16 overflow-hidden'>
        <GoogleMap
          mapContainerStyle={containerStyle}
          center={mapCenter}
          zoom={stops.length > 0 ? 11 : 12}
          onLoad={onMapLoad}
          options={{
            fullscreenControl: true,
            streetViewControl: false,
            mapTypeControl: false,
          }}
        >
          {directions ? (
            <DirectionsRenderer
              directions={directions}
              options={{
                suppressMarkers: true,
                polylineOptions: { strokeColor: '#2563eb', strokeWeight: 5, strokeOpacity: 0.8 },
              }}
            />
          ) : polylineCoords.length >= 2 ? (
            <Polyline
              path={polylineCoords}
              options={{
                strokeColor: '#2563eb',
                strokeWeight: 4,
                strokeOpacity: 0.7,
                geodesic: true,
              }}
            />
          ) : null}

          {/* Bus Current Position Marker */}
          {busPosition && (
            <Marker
              position={busPosition}
              label={{
                text: '🚌',
                fontSize: '28px',
              }}
              icon={{
                url: 'data:image/svg+xml;charset=UTF-8,<svg xmlns="http://www.w3.org/2000/svg" width="1" height="1"/>',
                scaledSize: { width: 1, height: 1 },
              }}
              title='Live Bus Location'
              onClick={() => setSelectedMarker({ type: 'bus', position: busPosition })}
            />
          )}

          {/* Route Stops Markers — white circle dot like Google Maps */}
          {stops.map((s, i) => (
            <Marker
              key={s.id || i}
              position={{ lat: parseFloat(s.latitude), lng: parseFloat(s.longitude) }}
              title={s.name}
              icon={{
                url: `data:image/svg+xml;charset=UTF-8,${encodeURIComponent(
                  '<svg xmlns="http://www.w3.org/2000/svg" width="18" height="18" viewBox="0 0 18 18">' +
                  '<circle cx="9" cy="9" r="7" fill="white" stroke="#2563eb" stroke-width="2.5"/>' +
                  '</svg>'
                )}`,
                scaledSize: { width: 18, height: 18 },
                anchor: { x: 9, y: 9 },
              }}
              onClick={() => setSelectedMarker({ type: 'stop', data: s, index: i + 1 })}
            />
          ))}

          {/* InfoWindow for selected Marker */}
          {selectedMarker && selectedMarker.type === 'stop' && (
            <InfoWindow
              position={{
                lat: parseFloat(selectedMarker.data.latitude),
                lng: parseFloat(selectedMarker.data.longitude),
              }}
              onCloseClick={() => setSelectedMarker(null)}
            >
              <div className='p-1 max-w-xs'>
                <p className='font-bold text-sm text-stone-800'>
                  Stop #{selectedMarker.index}: {selectedMarker.data.name}
                </p>
                <p className='text-xs text-gray-500 mt-0.5'>
                  Lat: {parseFloat(selectedMarker.data.latitude).toFixed(4)}, Lng: {parseFloat(selectedMarker.data.longitude).toFixed(4)}
                </p>
              </div>
            </InfoWindow>
          )}

          {selectedMarker && selectedMarker.type === 'bus' && (
            <InfoWindow
              position={selectedMarker.position}
              onCloseClick={() => setSelectedMarker(null)}
            >
              <div className='p-1 max-w-xs'>
                <p className='font-bold text-sm text-lime-800'>🚌 Bus Location</p>
                <p className='text-xs text-gray-600 mt-1'>
                  Speed: {busSpeed.toFixed(1)} km/h
                </p>
                {lastUpdate && <p className='text-[10px] text-gray-400'>Updated: {lastUpdate}</p>}
              </div>
            </InfoWindow>
          )}
        </GoogleMap>

        {/* Bottom sheet */}
        <div
          className='absolute bottom-0 left-0 right-0 bg-white rounded-t-2xl shadow-2xl transition-[height] duration-150 ease-out z-10'
          style={{ height }}
        >
          <div
            onPointerDown={handlePointerDown}
            onPointerMove={handlePointerMove}
            onPointerUp={handlePointerUp}
            className='w-full flex justify-center py-3 touch-none cursor-grab active:cursor-grabbing'
          >
            <div className='w-10 h-1.5 bg-gray-300 rounded-full' />
          </div>

          <div className='px-4 pb-4 overflow-y-auto space-y-4' style={{ height: height - 40 }}>
            <div className='border border-gray-200 rounded-xl p-4 space-y-3'>
              <div className='flex items-center justify-between'>
                <div className='flex items-center gap-2'>
                  <Bus size={18} className='text-stone-700' />
                  <span className='font-semibold text-base'>
                    {busId ? `Bus ${busId.slice(0, 8)}...` : 'Bus Tracking'}
                  </span>
                </div>
                <div className={`flex items-center gap-1 text-xs px-2 py-1 rounded-full ${connected ? 'bg-green-100 text-green-700 font-medium' : 'bg-gray-100 text-gray-500'}`}>
                  {connected ? <Wifi size={12} /> : <WifiOff size={12} />}
                  {connected ? 'Live GPS' : 'Offline'}
                </div>
              </div>

              <div className='text-sm text-gray-700 font-medium flex items-center gap-2'>
                <Navigation size={14} className='text-lime-700' />
                <span>{from}</span>
                <span className='mx-1 text-gray-400'>→</span>
                <span>{to}</span>
              </div>

              <div className='flex items-center justify-between bg-stone-50 rounded-lg px-3 py-2'>
                <div className='flex items-center gap-2 text-sm text-gray-700'>
                  <Clock size={16} /> Estimated Arrival
                </div>
                <span className='text-sm font-semibold text-lime-800'>{estimateETA()}</span>
              </div>

              <div className='flex items-center justify-between bg-stone-50 rounded-lg px-3 py-2'>
                <div className='flex items-center gap-2 text-sm text-gray-700'>
                  <Gauge size={16} /> Current Speed
                </div>
                <span className='text-sm font-medium'>{busSpeed.toFixed(1)} km/h</span>
              </div>

              {stops.length > 0 && (
                <div className='flex items-center justify-between bg-stone-50 rounded-lg px-3 py-2'>
                  <div className='flex items-center gap-2 text-sm text-gray-700'>
                    <MapPin size={16} /> Destination Stop
                  </div>
                  <span className='text-sm font-medium'>{nextStop}</span>
                </div>
              )}

              {lastUpdate && (
                <div className='flex items-center justify-between bg-stone-50 rounded-lg px-3 py-2'>
                  <div className='flex items-center gap-2 text-sm text-gray-700'>
                    <AlertTriangle size={16} /> Last GPS Update
                  </div>
                  <span className='text-sm font-medium text-gray-600'>{lastUpdate}</span>
                </div>
              )}

              {stops.length > 0 && (
                <div className='border-t border-gray-100 pt-3'>
                  <div className='flex items-center gap-2 text-xs font-semibold text-gray-500 uppercase tracking-wider mb-2'>
                    <MapPin size={14} /> Route Stops ({stops.length})
                  </div>
                  <div className='space-y-1.5'>
                    {stops.map((s, idx) => (
                      <div key={s.id || idx} className='flex items-center gap-2 text-sm text-gray-700 pl-1'>
                        <span className='w-5 h-5 rounded-full bg-lime-100 text-lime-800 text-[11px] font-bold flex items-center justify-center shrink-0'>
                          {idx + 1}
                        </span>
                        <span className='font-medium'>{s.name}</span>
                        <span className='text-[10px] text-gray-400 ml-auto'>
                          {parseFloat(s.latitude).toFixed(3)}, {parseFloat(s.longitude).toFixed(3)}
                        </span>
                      </div>
                    ))}
                  </div>
                </div>
              )}
            </div>

            {/* Bus Details & Live Ratings card */}
            <div className='border border-gray-200 rounded-xl p-4 space-y-4'>
              <div className='flex items-center justify-between'>
                <h3 className='font-semibold text-base text-gray-800'>Passenger Ratings & Feedback</h3>
                <button
                  type='button'
                  onClick={() => setShowFeedbackModal(true)}
                  className='flex items-center gap-1.5 bg-lime-700 hover:bg-lime-800 text-white text-xs font-semibold px-3 py-1.5 rounded-lg shadow-sm transition'
                >
                  <MessageSquare size={13} />
                  Rate & Review
                </button>
              </div>

              {/* Rating aggregate display */}
              <div className='flex items-center gap-3 bg-amber-50/60 p-3 rounded-xl border border-amber-100'>
                <div className='flex flex-col items-center justify-center bg-white px-3 py-2 rounded-lg border border-amber-200/80 shadow-xs'>
                  <span className='text-2xl font-black text-amber-600'>
                    {feedbackData.average_rating ? Number(feedbackData.average_rating).toFixed(1) : '5.0'}
                  </span>
                  <div className='flex gap-0.5 mt-0.5'>
                    {[1, 2, 3, 4, 5].map((n) => (
                      <Star
                        key={n}
                        size={11}
                        className={
                          n <= Math.round(feedbackData.average_rating || 5)
                            ? 'fill-amber-500 text-amber-500'
                            : 'text-gray-300'
                        }
                      />
                    ))}
                  </div>
                </div>

                <div className='flex-1 space-y-1'>
                  <div className='text-xs font-bold text-gray-700'>
                    {feedbackData.total_reviews || 0} Passenger Reviews
                  </div>
                  <div className='grid grid-cols-2 gap-2 text-[11px] text-gray-600 mt-1'>
                    <div className='flex items-center gap-1 bg-white px-2 py-1 rounded border border-gray-100'>
                      <Sparkles size={12} className='text-teal-600 shrink-0' />
                      <span>Clean: <strong>{Number(feedbackData.avg_cleanliness || 5).toFixed(1)}/5</strong></span>
                    </div>
                    <div className='flex items-center gap-1 bg-white px-2 py-1 rounded border border-gray-100'>
                      <Wind size={12} className='text-sky-600 shrink-0' />
                      <span>AC: <strong>{feedbackData.ac_working_pct ?? 100}%</strong></span>
                    </div>
                  </div>
                </div>
              </div>

              {/* Status Pills */}
              <div className='flex items-center gap-2 text-xs'>
                <span className={`px-2.5 py-1 rounded-full font-medium flex items-center gap-1 ${
                  (feedbackData.crowded_pct || 0) > 50
                    ? 'bg-orange-100 text-orange-800'
                    : 'bg-emerald-100 text-emerald-800'
                }`}>
                  <Users size={12} />
                  {(feedbackData.crowded_pct || 0) > 50 ? 'Likely Crowded' : 'Seats Comfortable'}
                </span>
                <span className='px-2.5 py-1 rounded-full font-medium bg-stone-100 text-stone-700 flex items-center gap-1'>
                  <Wind size={12} />
                  {(feedbackData.ac_working_pct ?? 100) >= 60 ? 'AC Reported On' : 'AC Issue Reported'}
                </span>
              </div>

              {/* Vehicle & Driver Info */}
              <div className='border-t border-gray-100 pt-3 space-y-2 text-xs text-gray-600'>
                <div className='flex items-center justify-between'>
                  <span className='flex items-center gap-1.5 text-gray-500'>
                    <User size={14} /> Driver
                  </span>
                  <span className='font-medium text-gray-800'>{staticInfo.driver}</span>
                </div>
                <div className='flex items-center justify-between'>
                  <span className='flex items-center gap-1.5 text-gray-500'>
                    <Phone size={14} /> Helpline
                  </span>
                  <span className='font-medium text-gray-800'>{staticInfo.contact}</span>
                </div>
              </div>

              {/* Recent passenger feedback comments */}
              {feedbackData.recent_comments && feedbackData.recent_comments.length > 0 && (
                <div className='border-t border-gray-100 pt-3'>
                  <div className='flex items-center gap-1.5 text-xs font-semibold text-gray-600 uppercase tracking-wider mb-2'>
                    <MessageSquare size={13} /> Recent Passenger Feedback
                  </div>
                  <div className='space-y-2'>
                    {feedbackData.recent_comments.map((c, idx) => (
                      <div key={idx} className='bg-stone-50 rounded-lg p-2.5 text-xs border border-stone-200/60'>
                        <div className='flex items-center justify-between mb-1'>
                          <span className='font-medium text-gray-800'>Passenger</span>
                          <span className='text-[10px] text-gray-400'>
                            {c.created_at ? new Date(c.created_at).toLocaleDateString() : 'Recent'}
                          </span>
                        </div>
                        <p className='text-gray-700 italic'>"{c.comment}"</p>
                        <div className='flex items-center gap-2 mt-1.5 text-[10px] text-gray-500'>
                          <span>Cleanliness: {c.cleanliness}/5</span>
                          <span>•</span>
                          <span>AC: {c.ac_working ? 'Yes' : 'No'}</span>
                          <span>•</span>
                          <span>{c.is_crowded ? 'Crowded' : 'Not Crowded'}</span>
                        </div>
                      </div>
                    ))}
                  </div>
                </div>
              )}
            </div>

            {wsError && (
              <div className='border border-red-200 rounded-xl p-3 bg-red-50 text-sm text-red-700'>
                {wsError}
              </div>
            )}
          </div>
        </div>

        {/* Modal for Feedback Submission */}
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

      </div>
      <Footer />
    </div>
  )
}

export default Track
