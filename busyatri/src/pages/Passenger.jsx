import React, { useState, useEffect } from 'react'
import Navbar from '../components/Navbar'
import BusRow from '../components/Busrow'
import Footer from '../components/Footer'
import { MapPin, CalendarDays, AlertCircle, Loader2 } from 'lucide-react'
import { useSearchParams, useNavigate } from 'react-router-dom'
import { routeService } from '../services/routeService'

const Passenger = () => {
  const navigate = useNavigate()
  const [searchParams] = useSearchParams()
  const from = searchParams.get('from') || 'Location 1'
  const to = searchParams.get('to') || 'Location 2'
  const rawDate = searchParams.get('date')
  const date = rawDate
    ? new Date(rawDate).toLocaleDateString('en-GB', { day: '2-digit', month: 'short', year: 'numeric' })
    : new Date().toLocaleDateString('en-GB', { day: '2-digit', month: 'short', year: 'numeric' })

  const [buses, setBuses] = useState([])
  const [matchedRoutes, setMatchedRoutes] = useState([])
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState(null)

  useEffect(() => {
    const fetchData = async () => {
      setLoading(true)
      setError(null)
      try {
        const routesData = await routeService.getRoutes().catch(() => [])

        const isDefaultSearch =
          (!from || from === 'Location 1') && (!to || to === 'Location 2')

        const fromLower = (from && from !== 'Location 1' ? from : '').toLowerCase().trim()
        const toLower = (to && to !== 'Location 2' ? to : '').toLowerCase().trim()

        const matched = Array.isArray(routesData)
          ? routesData.filter((r) => {
              if (isDefaultSearch) return true
              const matchOrigin = !fromLower || r.origin?.toLowerCase().includes(fromLower)
              const matchDest = !toLower || r.destination?.toLowerCase().includes(toLower)
              return matchOrigin && matchDest
            })
          : []
        setMatchedRoutes(matched)

        // Fetch assigned buses for matched routes, filtered by selected date
        const busesPerRoute = await Promise.all(
          matched.map((route) =>
            routeService
              .getRouteBuses(route.id, rawDate || '')
              .then((data) =>
                Array.isArray(data)
                  ? data.map((b) => ({ ...b, routeId: route.id, routeName: route.name, routeOrigin: route.origin, routeDest: route.destination }))
                  : []
              )
              .catch(() => [])
          )
        )
        let foundBuses = busesPerRoute.flat()

        // If no buses assigned on that date, show nothing (don't fall back to unfiltered fleet)
        setBuses(foundBuses)
      } catch (err) {
        setError(err.message || 'Failed to fetch data')
      } finally {
        setLoading(false)
      }
    }
    fetchData()
  }, [from, to, rawDate])

  return (
    <div className='min-h-screen bg-amber-50 flex flex-col'>
      <Navbar />

      <div className='flex-1 pt-20 px-4 md:px-14 pb-10'>
        {/* Search summary bar */}
        <div className='mx-auto max-w-5xl rounded-xl bg-white p-4 md:p-5 shadow-sm flex flex-col md:flex-row items-stretch md:items-center gap-3 divide-y md:divide-y-0 md:divide-x divide-gray-200'>
          <div className='flex-1 flex items-center gap-2 py-2 md:py-0 px-1 md:px-3'>
            <MapPin size={18} className='text-gray-500 shrink-0' />
            <div>
              <p className='text-[10px] font-semibold text-gray-500'>FROM</p>
              <p className='text-sm text-gray-800'>{from}</p>
            </div>
          </div>
          <div className='flex-1 flex items-center gap-2 py-2 md:py-0 px-1 md:px-3'>
            <MapPin size={18} className='text-gray-500 shrink-0' />
            <div>
              <p className='text-[10px] font-semibold text-gray-500'>TO</p>
              <p className='text-sm text-gray-800'>{to}</p>
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

        {/* Bus listing */}
        <div className='mx-auto mt-6 max-w-5xl rounded-xl border border-gray-200 bg-white overflow-hidden'>
          <div className='flex flex-col gap-1 px-6 py-4 md:flex-row md:items-baseline md:justify-between'>
            <h2 className='text-sm font-semibold text-gray-500'>LIST OF BUSES</h2>
            <div className='text-sm text-gray-800'>
              <span className='font-medium'>{from} ⇄ {to}</span>
              <span className='block text-xs text-gray-400 md:ml-3 md:inline'>{date}</span>
            </div>
          </div>

          {loading ? (
            <div className='flex items-center justify-center py-16 text-gray-500'>
              <Loader2 size={24} className='animate-spin mr-2' />
              Searching for buses...
            </div>
          ) : error ? (
            <div className='flex items-center justify-center py-16 text-red-500'>
              <AlertCircle size={20} className='mr-2' />
              {error}
            </div>
          ) : buses.length === 0 ? (
            <div className='text-center py-16 text-gray-500'>
              <p className='text-lg font-medium'>No buses found</p>
              <p className='text-sm mt-1'>
                {matchedRoutes.length === 0
                  ? 'No route matches this From/To — try different wording'
                  : 'A matching route exists, but no bus is assigned to it yet'}
              </p>
            </div>
          ) : (
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
                    key={bus.id}
                    index={i + 1}
                    busNo={bus.license_plate}
                    arrivalTime={`${bus.capacity} seats`}
                    rating={bus.rating ? Math.round(bus.rating) : (bus.status === 'ACTIVE' ? 5 : 3)}
                    onTrack={() => {
                      const originName = bus.routeOrigin || from
                      const destName = bus.routeDest || to
                      navigate(`/track?busId=${bus.id}&routeId=${bus.routeId || ''}&from=${encodeURIComponent(originName)}&to=${encodeURIComponent(destName)}`)
                    }}
                  />
                ))}
              </div>
            </div>
          )}
        </div>

        {/* Matched routes info */}
        {matchedRoutes.length > 0 && (
          <div className='mx-auto mt-4 max-w-5xl'>
            <p className='text-xs text-gray-500'>
              Found {matchedRoutes.length} matching route(s): {matchedRoutes.map((r) => r.name).join(', ')}
            </p>
          </div>
        )}
      </div>
      <Footer />
    </div>
  )
}

export default Passenger
