import React, { useState, useRef, useEffect } from 'react'
import Navbar from '../components/Navbar'
import Footer from '../components/Footer'
import { useSearchParams } from 'react-router-dom'
import { GoogleMap, Marker, DirectionsRenderer, useJsApiLoader } from '@react-google-maps/api'
import { Bus, Clock, AlertTriangle, MapPin, Gauge, User, Phone, Star } from 'lucide-react'

const GOOGLE_MAPS_API_KEY = 'AIzaSyBai8C4ydAkEYYeVCEatMXkUi_BeS0OCKg'

const routeCoords = [
  { lat: 28.6139, lng: 77.2090 },
  { lat: 28.4089, lng: 77.3178 },
  { lat: 27.1767, lng: 78.0081 },
  { lat: 26.4499, lng: 80.3319 },
]

const containerStyle = {
  width: '100%',
  height: '100%',
}

const SNAP_POINTS = [120, 400, 620]

const staticInfo = {
  via: 'Yamuna Expressway',
  eta: '3h 45min',
  delay: '12min',
  nextStop: 'Agra',
  nextStopDistance: '50KM Away',
  speed: '60 km/h',
  driver: 'Name',
  contact: '0000000000',
  rating: 5,
}

const Track = () => {
  const [searchParams] = useSearchParams()
  const from = searchParams.get('from') || 'Location 1'
  const to = searchParams.get('to') || 'Location 2'
  const busNo = searchParams.get('busNo') || 'N/A'

  const { isLoaded } = useJsApiLoader({
    googleMapsApiKey: GOOGLE_MAPS_API_KEY,
  })

  const [height, setHeight] = useState(SNAP_POINTS[0])
  const [directions, setDirections] = useState(null)
  const startY = useRef(0)
  const startHeight = useRef(0)

  useEffect(() => {
    if (!isLoaded) return

    const directionsService = new window.google.maps.DirectionsService()
    const origin = routeCoords[0]
    const destination = routeCoords[routeCoords.length - 1]
    const waypoints = routeCoords.slice(1, -1).map((point) => ({
      location: point,
      stopover: false,
    }))

    directionsService.route(
      {
        origin,
        destination,
        waypoints,
        travelMode: window.google.maps.TravelMode.DRIVING,
      },
      (result, status) => {
        if (status === 'OK') {
          setDirections(result)
        }
      }
    )
  }, [isLoaded])

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

  if (!isLoaded) return <div className='flex items-center justify-center bg-lime-100 h-screen'>Loading map...</div>

  return (
    <div className='flex min-h-screen flex-col'>
      <Navbar />
      <div className='relative h-[calc(100vh-110px)] w-full mt-16 overflow-hidden'>
        <GoogleMap mapContainerStyle={containerStyle} center={routeCoords[1]} zoom={8}>
          {directions && (
            <DirectionsRenderer
              directions={directions}
              options={{
                suppressMarkers: true,
                polylineOptions: {
                  strokeColor: 'blue',
                  strokeWeight: 4,
                },
              }}
            />
          )}
          <Marker position={routeCoords[1]} />
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
              <div className='flex items-center gap-2'>
                <Bus size={18} className='text-stone-700' />
                <span className='font-semibold text-base'>{busNo}</span>
              </div>
              <div className='text-sm text-gray-600'>
                {from} <span className='mx-1'>⇄</span> {to}
                <div className='text-xs text-gray-400'>via {staticInfo.via}</div>
              </div>

              <div className='flex items-center justify-between bg-stone-50 rounded-lg px-3 py-2'>
                <div className='flex items-center gap-2 text-sm text-gray-700'>
                  <Clock size={16} />
                  ETA
                </div>
                <span className='text-sm font-medium'>{staticInfo.eta}</span>
              </div>

              <div className='flex items-center justify-between bg-stone-50 rounded-lg px-3 py-2'>
                <div className='flex items-center gap-2 text-sm text-gray-700'>
                  <AlertTriangle size={16} />
                  Delayed by
                </div>
                <span className='text-sm font-medium text-red-500'>{staticInfo.delay}</span>
              </div>

              <div className='flex items-center justify-between bg-stone-50 rounded-lg px-3 py-2'>
                <div className='flex items-center gap-2 text-sm text-gray-700'>
                  <MapPin size={16} />
                  Next Stop
                </div>
                <div className='text-right'>
                  <div className='text-sm font-medium'>{staticInfo.nextStop}</div>
                  <div className='text-xs text-gray-400'>{staticInfo.nextStopDistance}</div>
                </div>
              </div>

              <div className='flex items-center justify-between bg-stone-50 rounded-lg px-3 py-2'>
                <div className='flex items-center gap-2 text-sm text-gray-700'>
                  <Gauge size={16} />
                  Speed
                </div>
                <span className='text-sm font-medium'>{staticInfo.speed}</span>
              </div>
            </div>

            <div className='border border-gray-200 rounded-xl p-4 space-y-3'>
              <h3 className='font-semibold text-base'>Bus Details</h3>

              <div className='flex items-center gap-2 text-sm text-gray-700'>
                <User size={16} />
                Driver: {staticInfo.driver}
              </div>

              <div className='flex items-center gap-2 text-sm text-gray-700'>
                <Phone size={16} />
                Contact: {staticInfo.contact}
              </div>

              <div className='flex items-center gap-2'>
              {[1, 2, 3, 4, 5].map((n) => (
                <Star
                  key={n}
                  size={16}
                  className={n <= staticInfo.rating ? 'fill-lime-800 text-lime-800' : 'text-gray-300'}
                />
              ))}
              </div>
            </div>
          </div>
        </div>
      </div>
      <Footer />
    </div>
  )
}

export default Track