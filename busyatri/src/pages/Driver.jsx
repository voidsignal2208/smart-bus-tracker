import { useEffect, useRef, useState } from 'react'
import { Link, useNavigate } from 'react-router-dom'
import { BusFront, LogOut, MapPin, Square } from 'lucide-react'
import Footer from '../components/Footer'
import { api } from '../services/api'
import { useAuth } from '../context/AuthContext'

const Driver = () => {
  const navigate = useNavigate()
  const { logout } = useAuth()
  const watchId = useRef(null)
  const [assignment, setAssignment] = useState(null)
  const [sharing, setSharing] = useState(false)
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState('')
  const [updatedAt, setUpdatedAt] = useState(null)

  useEffect(() => {
    api.get('/api/v1/fleet/my-assignment').then(setAssignment).catch((err) => setError(err.message || 'Could not load your bus assignment.')).finally(() => setLoading(false))
    return () => { if (watchId.current !== null) navigator.geolocation?.clearWatch(watchId.current) }
  }, [])

  const stopSharing = () => { if (watchId.current !== null) navigator.geolocation.clearWatch(watchId.current); watchId.current = null; setSharing(false) }
  const sendLocation = async (position) => {
    await api.post(`/api/v1/tracking/buses/${assignment.bus_id}/location`, { latitude: position.coords.latitude, longitude: position.coords.longitude, speed_kmh: position.coords.speed == null ? undefined : position.coords.speed * 3.6 })
    setUpdatedAt(new Date()); setError('')
  }
  const startSharing = () => {
    if (!assignment?.bus_id) { setError('No bus is currently assigned to your account. Contact an administrator.'); return }
    if (!navigator.geolocation) { setError('This browser does not support location sharing.'); return }
    setError('')
    watchId.current = navigator.geolocation.watchPosition(
      (position) => sendLocation(position).catch((err) => { stopSharing(); setError(err.message || 'Could not send location.') }),
      (geoError) => { stopSharing(); setError(geoError.message || 'Could not read your location.') },
      { enableHighAccuracy: true, maximumAge: 10000, timeout: 20000 },
    )
    setSharing(true)
  }
  const handleLogout = () => { stopSharing(); logout(); navigate('/') }

  return <div className="min-h-screen bg-amber-50 flex flex-col">
    <header className="w-full h-16 px-4 md:px-8 flex justify-between items-center bg-white border-b border-gray-100"><h1 className="flex items-center gap-1 text-black text-xl font-medium"><BusFront size={26} />Bus<span className="text-lime-700 font-semibold">Yatri</span></h1><div className="flex items-center gap-4"><Link to="/" className="text-sm text-gray-800 hover:underline">Home</Link><button onClick={handleLogout} className="flex items-center gap-2 bg-gray-900 text-white text-sm font-medium rounded-lg px-4 py-2"><LogOut size={16} />Logout</button></div></header>
    <main className="flex-1 w-full max-w-3xl mx-auto px-4 md:px-6 py-8 flex flex-col gap-6">
      <section className="bg-white rounded-2xl shadow-sm p-6"><p className="text-xs font-semibold text-gray-400 uppercase tracking-wide mb-3">Assigned bus</p>{loading ? <p className="text-sm text-gray-500">Loading your assignment…</p> : assignment ? <div><p className="text-xl font-bold text-gray-900">{assignment.license_plate}</p><p className="mt-1 text-sm text-gray-600">Driver: {assignment.driver_name}{assignment.route_name ? ` · Route: ${assignment.route_name}` : ''}</p><p className="mt-2 text-xs text-gray-500">{assignment.capacity} seats · {assignment.status}</p></div> : <p className="text-sm text-amber-700">No bus is assigned to you yet. An administrator must assign your bus.</p>}</section>
      <section className="bg-white rounded-2xl shadow-sm p-6"><p className="text-xs font-semibold text-gray-400 uppercase tracking-wide mb-5">Live location</p><div className="flex items-center justify-between gap-4 flex-wrap"><div className="flex items-start gap-3"><span className={`mt-1.5 h-2.5 w-2.5 rounded-full ${sharing ? 'bg-green-500' : 'bg-gray-300'}`} /><div><p className="font-semibold text-gray-900">{sharing ? 'Location active' : 'Location paused'}</p><p className="text-sm text-gray-500">{assignment ? `Sharing updates for ${assignment.license_plate}` : 'Your assigned bus will appear here'}</p>{updatedAt && <p className="text-xs text-gray-400 mt-1">Last sent at {updatedAt.toLocaleTimeString()}</p>}</div></div><button disabled={!assignment || loading} onClick={sharing ? stopSharing : startSharing} className="flex items-center gap-2 bg-gray-900 disabled:opacity-40 text-white text-sm font-medium rounded-lg px-4 py-2"><Square size={14} />{sharing ? 'Stop sharing' : 'Start sharing'}</button></div></section>
      {error && <div className="bg-red-50 text-red-700 rounded-xl p-4 text-sm flex gap-2"><MapPin size={18} />{error}</div>}
    </main><Footer />
  </div>
}

export default Driver
