import React, { useEffect, useState } from 'react'
import Navbar from '../components/Navbar'
import BusRow from '../components/Busrow'
import Footer from '../components/Footer'
import { MapPin, CalendarDays, Info, Loader2 } from 'lucide-react'
import { useSearchParams, useNavigate } from 'react-router-dom'
import { getRoutes, getBuses, getRouteBuses, ApiError } from '../lib/api'

const Passenger = () => {
  const navigate = useNavigate()
  const [searchParams] = useSearchParams()
  const from = searchParams.get('from') || ''
  const to = searchParams.get('to') || ''
  const rawDate = searchParams.get('date')
  const date = rawDate
    ? new Date(rawDate).toLocaleDateString('en-GB', { day: '2-digit', month: 'short', year: 'numeric' })
    : new Date().toLocaleDateString('en-GB', { day: '2-digit', month: 'short', year: 'numeric' })

  const [loading, setLoading] = useState(true)
  const [error, setError] = useState('')
  const [matchedRoutes, setMatchedRoutes] = useState([])
  // Each entry: { id, license_plate, capacity, status, route_id, route_name }
  // route_id/route_name are only present when this bus was resolved via a
  // matched route below - otherwise the "all buses" fallback list is shown
  // and Track.jsx just won't have a route to draw.
  const [buses, setBuses] = useState([])

  useEffect(() => {
    let cancelled = false

    async function load() {
      setLoading(true)
      setError('')
      try {
        // Real data from the backend - both endpoints require a logged-in
        // user, which ProtectedRoute already guarantees before this page
        // renders.
        const allRoutes = await getRoutes()
        if (cancelled) return

        const term = (s) => s.trim().toLowerCase()
        const filtered = allRoutes.filter((route) => {
          const matchesFrom = !from || term(route.origin).includes(term(from))
          const matchesTo = !to || term(route.destination).includes(term(to))
          return matchesFrom && matchesTo
        })
        setMatchedRoutes(filtered)

        if (filtered.length > 0) {
          // A specific from/to search matched one or more routes - resolve
          // each to the buses actually assigned to it (bus_assignments),
          // via GET /api/v1/routes/{routeId}/buses. This is what lets a
          // "Track" click carry a real route_id through to the map, instead
          // of every bus in the fleet showing up regardless of route.
          const perRoute = await Promise.all(
            filtered.map((route) =>
              getRouteBuses(route.id).then((busesOnRoute) =>
                busesOnRoute.map((b) => ({ ...b, route_id: route.id, route_name: route.name }))
              )
            )
          )
          if (cancelled) return
          setBuses(perRoute.flat())
        } else {
          // No route matched this search (or no search was made) - fall
          // back to listing the whole fleet, same as before.
          const allBuses = await getBuses()
          if (cancelled) return
          setBuses(allBuses)
        }
      } catch (err) {
        if (!cancelled) {
          setError(err instanceof ApiError ? err.message : 'Failed to load buses and routes.')
        }
      } finally {
        if (!cancelled) setLoading(false)
      }
    }

    load()
    return () => {
      cancelled = true
    }
  }, [from, to])

  return (
    <div className='min-h-screen bg-amber-50 flex flex-col'>
      <Navbar />

      <div className='flex-1 pt-20 px-4 md:px-14 pb-10'>
        <div className='mx-auto max-w-5xl rounded-xl bg-white p-4 md:p-5 shadow-sm flex flex-col md:flex-row items-stretch md:items-center gap-3 divide-y md:divide-y-0 md:divide-x divide-gray-200'>
          <div className='flex-1 flex items-center gap-2 py-2 md:py-0 px-1 md:px-3'>
            <MapPin size={18} className='text-gray-500 shrink-0' />
            <div>
              <p className='text-[10px] font-semibold text-gray-500'>FROM</p>
              <p className='text-sm text-gray-800'>{from || 'Any origin'}</p>
            </div>
          </div>

          <div className='flex-1 flex items-center gap-2 py-2 md:py-0 px-1 md:px-3'>
            <MapPin size={18} className='text-gray-500 shrink-0' />
            <div>
              <p className='text-[10px] font-semibold text-gray-500'>TO</p>
              <p className='text-sm text-gray-800'>{to || 'Any destination'}</p>
            </div>
          </div>

          <div className='flex-1 flex items-center gap-2 py-2 md:py-0 px-1 md:px-3'>
            <CalendarDays size={18} className='text-gray-500 shrink-0' />
            <div>
              <p className='text-[10px] font-semibold text-gray-500'>DATE</p>
              <p className='text-sm text-gray-800'>{date}</p>
            </div>
          </div>
        </div>

        {matchedRoutes.length > 0 && (
          <div className='mx-auto mt-6 max-w-5xl rounded-xl border border-gray-200 bg-white px-6 py-4'>
            <h2 className='text-sm font-semibold text-gray-500 mb-2'>MATCHING ROUTES</h2>
            <div className='flex flex-wrap gap-2'>
              {matchedRoutes.map((route) => (
                <span
                  key={route.id}
                  className='rounded-full bg-lime-100 px-3 py-1 text-xs font-medium text-lime-900'
                >
                  {route.name}: {route.origin} → {route.destination}
                </span>
              ))}
            </div>
          </div>
        )}

        <div className='mx-auto mt-6 max-w-5xl rounded-xl border border-gray-200 bg-white overflow-hidden'>
          <div className='flex flex-col gap-1 px-6 py-4 md:flex-row md:items-baseline md:justify-between'>
            <h2 className='text-sm font-semibold text-gray-500'>FLEET</h2>
            <div className='text-sm text-gray-800'>
              {from && to ? (
                <span className='font-medium'>{from} ⇄ {to}</span>
              ) : (
                <span className='font-medium'>All active buses</span>
              )}
            </div>
          </div>

          {(from || to) && matchedRoutes.length === 0 && (
            <div className='mx-6 mb-3 flex items-start gap-2 rounded-lg bg-amber-50 px-3 py-2 text-xs text-gray-600'>
              <Info size={14} className='mt-0.5 shrink-0' />
              <span>
                No route matched that search, so every active bus in the fleet is listed
                below instead - pick any one to open its live tracking view.
              </span>
            </div>
          )}

          {loading && (
            <div className='flex items-center justify-center gap-2 py-10 text-gray-500'>
              <Loader2 className='animate-spin' size={18} /> Loading buses...
            </div>
          )}

          {!loading && error && (
            <div className='px-6 py-8 text-center text-sm text-red-600'>{error}</div>
          )}

          {!loading && !error && buses.length === 0 && (
            <div className='px-6 py-8 text-center text-sm text-gray-500'>
              No buses in the fleet yet.
            </div>
          )}

          {!loading && !error && buses.length > 0 && (
            <div className='overflow-x-auto'>
              <div className='min-w-160'>
                <div className='grid grid-cols-5 bg-stone-500 px-6 py-3 text-sm font-semibold text-white'>
                  <div>S No.</div>
                  <div>Bus No.</div>
                  <div>Capacity</div>
                  <div>Status</div>
                  <div>Track</div>
                </div>

                {buses.map((bus, i) => (
                  <BusRow
                    key={bus.route_id ? `${bus.route_id}-${bus.id}` : bus.id}
                    index={i + 1}
                    licensePlate={bus.license_plate}
                    routeName={bus.route_name}
                    capacity={bus.capacity}
                    status={bus.status}
                    onTrack={() => {
                      const params = new URLSearchParams({
                        busId: bus.id,
                        busNo: bus.license_plate,
                        from,
                        to,
                      })
                      // Only present when this bus was resolved through a
                      // matched route - Track.jsx draws the road route and
                      // stop markers when it's there, and falls back to a
                      // plain live-location view when it's not.
                      if (bus.route_id) params.set('routeId', bus.route_id)
                      navigate(`/track?${params.toString()}`)
                    }}
                  />
                ))}
              </div>
            </div>
          )}
        </div>
      </div>
      <Footer />
    </div>
  )
}

export default Passenger
