import React, { useEffect, useState } from 'react'
import Navbar from '../components/Navbar'
import Footer from '../components/Footer'
import { BusFront, Route as RouteIcon, Plus, Loader2 } from 'lucide-react'
import { getBuses, createBus, getRoutes, createRoute, ApiError } from '../lib/api'

const Admin = () => {
  const [buses, setBuses] = useState([])
  const [routes, setRoutes] = useState([])
  const [loading, setLoading] = useState(true)
  const [loadError, setLoadError] = useState('')

  const [busForm, setBusForm] = useState({ license_plate: '', capacity: '' })
  const [busSubmitting, setBusSubmitting] = useState(false)
  const [busError, setBusError] = useState('')

  const [routeForm, setRouteForm] = useState({ name: '', origin: '', destination: '' })
  const [routeSubmitting, setRouteSubmitting] = useState(false)
  const [routeError, setRouteError] = useState('')

  const reload = async () => {
    setLoading(true)
    setLoadError('')
    try {
      const [b, r] = await Promise.all([getBuses(), getRoutes()])
      setBuses(b)
      setRoutes(r)
    } catch (err) {
      setLoadError(err instanceof ApiError ? err.message : 'Failed to load fleet data.')
    } finally {
      setLoading(false)
    }
  }

  useEffect(() => {
    reload()
  }, [])

  const handleCreateBus = async (e) => {
    e.preventDefault()
    setBusError('')
    setBusSubmitting(true)
    try {
      await createBus({
        license_plate: busForm.license_plate.trim(),
        capacity: Number(busForm.capacity),
      })
      setBusForm({ license_plate: '', capacity: '' })
      await reload()
    } catch (err) {
      setBusError(err instanceof ApiError ? err.message : 'Failed to create bus.')
    } finally {
      setBusSubmitting(false)
    }
  }

  const handleCreateRoute = async (e) => {
    e.preventDefault()
    setRouteError('')
    setRouteSubmitting(true)
    try {
      await createRoute({
        name: routeForm.name.trim(),
        origin: routeForm.origin.trim(),
        destination: routeForm.destination.trim(),
      })
      setRouteForm({ name: '', origin: '', destination: '' })
      await reload()
    } catch (err) {
      setRouteError(err instanceof ApiError ? err.message : 'Failed to create route.')
    } finally {
      setRouteSubmitting(false)
    }
  }

  return (
    <div className='min-h-screen bg-amber-50 flex flex-col'>
      <Navbar />

      <div className='flex-1 pt-20 px-4 md:px-14 pb-10'>
        <h1 className='text-2xl font-bold mb-6'>Admin Dashboard</h1>

        {loadError && <p className='mb-4 text-sm text-red-600'>{loadError}</p>}

        <div className='grid gap-6 md:grid-cols-2'>
          {/* --- Fleet --- */}
          <div className='rounded-xl bg-white p-5 shadow-sm'>
            <div className='mb-4 flex items-center gap-2'>
              <BusFront className='text-lime-800' size={22} />
              <h2 className='text-lg font-bold'>Fleet</h2>
            </div>

            <form onSubmit={handleCreateBus} className='mb-4 flex flex-col gap-2 sm:flex-row'>
              <input
                type='text'
                required
                placeholder='License plate (e.g. XYZ-1234)'
                value={busForm.license_plate}
                onChange={(e) => setBusForm({ ...busForm, license_plate: e.target.value })}
                className='flex-1 rounded border border-gray-300 px-3 py-2 text-sm outline-none'
                maxLength={20}
              />
              <input
                type='number'
                required
                min={1}
                max={200}
                placeholder='Capacity'
                value={busForm.capacity}
                onChange={(e) => setBusForm({ ...busForm, capacity: e.target.value })}
                className='w-full rounded border border-gray-300 px-3 py-2 text-sm outline-none sm:w-28'
              />
              <button
                type='submit'
                disabled={busSubmitting}
                className='flex items-center justify-center gap-1 rounded bg-lime-800 px-3 py-2 text-sm font-semibold text-white disabled:opacity-60'
              >
                <Plus size={16} /> Add
              </button>
            </form>
            {busError && <p className='mb-2 text-xs text-red-600'>{busError}</p>}

            {loading ? (
              <div className='flex items-center gap-2 text-sm text-gray-500'>
                <Loader2 className='animate-spin' size={16} /> Loading...
              </div>
            ) : buses.length === 0 ? (
              <p className='text-sm text-gray-500'>No buses yet.</p>
            ) : (
              <div className='divide-y divide-gray-100'>
                {buses.map((bus) => (
                  <div key={bus.id} className='flex items-center justify-between py-2 text-sm'>
                    <span className='font-medium'>{bus.license_plate}</span>
                    <span className='text-gray-500'>{bus.capacity} seats</span>
                    <span className='rounded-full bg-lime-100 px-2 py-0.5 text-xs text-lime-900'>{bus.status}</span>
                  </div>
                ))}
              </div>
            )}
          </div>

          {/* --- Routes --- */}
          <div className='rounded-xl bg-white p-5 shadow-sm'>
            <div className='mb-4 flex items-center gap-2'>
              <RouteIcon className='text-lime-800' size={22} />
              <h2 className='text-lg font-bold'>Routes</h2>
            </div>

            <form onSubmit={handleCreateRoute} className='mb-4 flex flex-col gap-2'>
              <input
                type='text'
                required
                placeholder='Route name (e.g. Airport Express)'
                value={routeForm.name}
                onChange={(e) => setRouteForm({ ...routeForm, name: e.target.value })}
                className='rounded border border-gray-300 px-3 py-2 text-sm outline-none'
                maxLength={100}
              />
              <div className='flex gap-2'>
                <input
                  type='text'
                  required
                  placeholder='Origin'
                  value={routeForm.origin}
                  onChange={(e) => setRouteForm({ ...routeForm, origin: e.target.value })}
                  className='flex-1 rounded border border-gray-300 px-3 py-2 text-sm outline-none'
                />
                <input
                  type='text'
                  required
                  placeholder='Destination'
                  value={routeForm.destination}
                  onChange={(e) => setRouteForm({ ...routeForm, destination: e.target.value })}
                  className='flex-1 rounded border border-gray-300 px-3 py-2 text-sm outline-none'
                />
              </div>
              <button
                type='submit'
                disabled={routeSubmitting}
                className='flex items-center justify-center gap-1 rounded bg-lime-800 px-3 py-2 text-sm font-semibold text-white disabled:opacity-60'
              >
                <Plus size={16} /> Add route
              </button>
            </form>
            {routeError && <p className='mb-2 text-xs text-red-600'>{routeError}</p>}

            {loading ? (
              <div className='flex items-center gap-2 text-sm text-gray-500'>
                <Loader2 className='animate-spin' size={16} /> Loading...
              </div>
            ) : routes.length === 0 ? (
              <p className='text-sm text-gray-500'>No routes yet.</p>
            ) : (
              <div className='divide-y divide-gray-100'>
                {routes.map((route) => (
                  <div key={route.id} className='py-2 text-sm'>
                    <span className='font-medium'>{route.name}</span>
                    <span className='text-gray-500'> — {route.origin} → {route.destination}</span>
                  </div>
                ))}
              </div>
            )}
          </div>
        </div>

        <p className='mt-6 text-xs text-gray-500'>
          To add stops to a route, or to promote a user to Driver/Conductor/Admin, use the
          backend API directly for now (see backend/README.md) - there's no UI for those yet.
        </p>
      </div>

      <Footer />
    </div>
  )
}

export default Admin
