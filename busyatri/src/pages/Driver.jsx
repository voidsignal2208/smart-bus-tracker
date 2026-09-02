import React, { useEffect, useRef, useState } from 'react'
import Navbar from '../components/Navbar'
import Footer from '../components/Footer'
import { BusFront, Navigation, Square, Loader2, WifiOff } from 'lucide-react'
import { getBuses, ApiError } from '../lib/api'
import { openDriverPushSocket } from '../lib/trackingSocket'

const Driver = () => {
  const [buses, setBuses] = useState([])
  const [loading, setLoading] = useState(true)
  const [loadError, setLoadError] = useState('')
  const [selectedBusId, setSelectedBusId] = useState('')

  const [tracking, setTracking] = useState(false)
  const [status, setStatus] = useState('idle') // idle | connecting | live | error
  const [statusMessage, setStatusMessage] = useState('')
  const [lastSent, setLastSent] = useState(null)

  const socketRef = useRef(null)
  const watchIdRef = useRef(null)

  useEffect(() => {
    let cancelled = false

    async function load() {
      setLoading(true)
      setLoadError('')
      try {
        const allBuses = await getBuses()
        if (!cancelled) {
          setBuses(allBuses)
          if (allBuses.length > 0) setSelectedBusId(allBuses[0].id)
        }
      } catch (err) {
        if (!cancelled) {
          setLoadError(err instanceof ApiError ? err.message : 'Failed to load the fleet list.')
        }
      } finally {
        if (!cancelled) setLoading(false)
      }
    }

    load()
    return () => {
      cancelled = true
    }
  }, [])

  // Always tear down the socket and the geolocation watch on unmount,
  // even if the user navigates away mid-shift instead of pressing Stop.
  useEffect(() => {
    return () => {
      if (watchIdRef.current !== null) navigator.geolocation.clearWatch(watchIdRef.current)
      socketRef.current?.close()
    }
  }, [])

  const startTracking = () => {
    if (!selectedBusId) return

    if (!('geolocation' in navigator)) {
      setStatus('error')
      setStatusMessage('This browser does not support geolocation.')
      return
    }

    setStatus('connecting')
    setStatusMessage('')

    const socket = openDriverPushSocket({
      onOpen: () => setStatus('live'),
      onAck: (payload) => {
        if (payload.ok === false) {
          setStatusMessage(payload.error || 'Server rejected the last location update.')
          return
        }
        setStatusMessage('')
        setLastSent({ latitude: payload.latitude, longitude: payload.longitude, speed_kmh: payload.speed_kmh, timestamp: payload.timestamp })
      },
      onError: (err) => {
        setStatus('error')
        setStatusMessage(err.message)
      },
    })
    socketRef.current = socket

    // Pushes a fresh reading every time the browser reports a
    // significantly new position (subject to the options below), not on
    // a fixed timer - watchPosition already coalesces redundant updates.
    watchIdRef.current = navigator.geolocation.watchPosition(
      (position) => {
        const speedMs = position.coords.speed // meters/second, may be null
        socket?.pushLocation(selectedBusId, {
          latitude: position.coords.latitude,
          longitude: position.coords.longitude,
          ...(speedMs != null ? { speed_kmh: speedMs * 3.6 } : {}),
        })
      },
      (geoErr) => {
        setStatus('error')
        setStatusMessage(geoErr.message)
      },
      { enableHighAccuracy: true, maximumAge: 4000, timeout: 10000 }
    )

    setTracking(true)
  }

  const stopTracking = () => {
    if (watchIdRef.current !== null) {
      navigator.geolocation.clearWatch(watchIdRef.current)
      watchIdRef.current = null
    }
    socketRef.current?.close()
    socketRef.current = null
    setTracking(false)
    setStatus('idle')
  }

  return (
    <div className='min-h-screen bg-amber-50 flex flex-col'>
      <Navbar />

      <div className='flex-1 pt-20 px-4 md:px-14 pb-10'>
        <h1 className='text-2xl font-bold mb-6'>Driver Console</h1>

        <div className='mx-auto max-w-lg rounded-xl bg-white p-5 shadow-sm'>
          <div className='mb-4 flex items-center gap-2'>
            <BusFront className='text-lime-800' size={22} />
            <h2 className='text-lg font-bold'>Push live location</h2>
          </div>

          {loading && (
            <div className='flex items-center gap-2 text-sm text-gray-500'>
              <Loader2 className='animate-spin' size={16} /> Loading fleet...
            </div>
          )}

          {loadError && <p className='text-sm text-red-600'>{loadError}</p>}

          {!loading && !loadError && buses.length === 0 && (
            <p className='text-sm text-gray-500'>No buses in the fleet yet - ask an admin to add one.</p>
          )}

          {!loading && buses.length > 0 && (
            <>
              <label className='mb-1 block text-xs text-gray-500'>BUS</label>
              <select
                value={selectedBusId}
                onChange={(e) => setSelectedBusId(e.target.value)}
                disabled={tracking}
                className='mb-4 w-full rounded border border-gray-300 px-3 py-2 text-sm outline-none disabled:bg-gray-100'
              >
                {buses.map((bus) => (
                  <option key={bus.id} value={bus.id}>
                    {bus.license_plate} ({bus.capacity} seats, {bus.status})
                  </option>
                ))}
              </select>

              {!tracking ? (
                <button
                  onClick={startTracking}
                  className='flex w-full items-center justify-center gap-2 rounded bg-lime-800 py-3 font-bold text-white'
                >
                  <Navigation size={18} /> Start sending location
                </button>
              ) : (
                <button
                  onClick={stopTracking}
                  className='flex w-full items-center justify-center gap-2 rounded bg-stone-700 py-3 font-bold text-white'
                >
                  <Square size={18} /> Stop
                </button>
              )}

              <div className='mt-4 flex items-center gap-2 text-sm'>
                {status === 'live' && <span className='h-2 w-2 rounded-full bg-lime-600' />}
                {status === 'connecting' && <Loader2 size={14} className='animate-spin' />}
                {status === 'error' && <WifiOff size={14} className='text-red-500' />}
                <span className='text-gray-600'>
                  {status === 'idle' && 'Not sending'}
                  {status === 'connecting' && 'Connecting...'}
                  {status === 'live' && 'Live - sending your position'}
                  {status === 'error' && (statusMessage || 'Connection error')}
                </span>
              </div>

              {lastSent && (
                <div className='mt-3 rounded-lg bg-stone-50 px-3 py-2 text-xs text-gray-600'>
                  Last sent: {lastSent.latitude.toFixed(5)}, {lastSent.longitude.toFixed(5)}
                  {lastSent.speed_kmh != null && <> · {Number(lastSent.speed_kmh).toFixed(1)} km/h</>}
                </div>
              )}

              <p className='mt-4 text-xs text-gray-500'>
                Your browser will ask for location permission. Keep this tab open while on
                shift - closing it (or pressing Stop) ends the live feed for this bus.
              </p>
            </>
          )}
        </div>
      </div>

      <Footer />
    </div>
  )
}

export default Driver
