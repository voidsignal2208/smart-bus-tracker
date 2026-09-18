import { useEffect, useState } from 'react'
import { useNavigate } from 'react-router-dom'
import { api } from '../services/api'
import { useAuth } from '../context/AuthContext'

const navItems = [
  { key: 'dashboard', label: 'Dashboard', icon: 'ti-layout-dashboard' },
  { key: 'createRoute', label: 'Create route', icon: 'ti-route' },
  { key: 'editRoute', label: 'Edit route', icon: 'ti-map-2' },
  { key: 'createBus', label: 'Create bus', icon: 'ti-square-rounded-plus' },
  { key: 'editBus', label: 'Edit bus', icon: 'ti-bus' },
  { key: 'track', label: 'Track buses', icon: 'ti-map-pin' },
]
const emptyPoint = { name: '', latitude: '', longitude: '' }
const emptyRoute = { name: '', start: emptyPoint, end: emptyPoint }
const emptyBus = { license_plate: '', capacity: '', status: 'ACTIVE', route_id: '', driver_id: '' }
const statuses = ['ACTIVE', 'MAINTENANCE', 'INACTIVE']
const fieldLabelClass = 'block text-[11px] font-semibold text-[#78716c] mb-[5px]'
const inputClass = 'w-full border border-[#e7e5e4] rounded-md px-[10px] py-2 text-[13px] outline-none focus:border-[#3f6212] bg-white'
const cardClass = 'bg-white rounded-xl p-5 shadow-[0_1px_2px_rgba(0,0,0,0.05)]'
const newStop = () => ({ name: '', latitude: '', longitude: '' })

const Admin = () => {
  const { logout } = useAuth()
  const navigate = useNavigate()
  const [view, setView] = useState('dashboard')
  const [buses, setBuses] = useState([])
  const [routes, setRoutes] = useState([])
  const [drivers, setDrivers] = useState([])
  const [routeForm, setRouteForm] = useState(emptyRoute)
  const [stops, setStops] = useState([])
  const [busForm, setBusForm] = useState(emptyBus)
  const [selectedRouteId, setSelectedRouteId] = useState('')
  const [selectedBusId, setSelectedBusId] = useState('')
  const [draggedStop, setDraggedStop] = useState(null)
  const [query, setQuery] = useState('')
  const [statusFilter, setStatusFilter] = useState('')
  const [loading, setLoading] = useState(true)
  const [saving, setSaving] = useState(false)
  const [message, setMessage] = useState('')
  const [error, setError] = useState('')

  const loadData = async () => {
    setLoading(true)
    try {
      const [busData, routeData, driverData] = await Promise.all([api.get('/api/v1/fleet/buses'), api.get('/api/v1/routes'), api.get('/api/v1/fleet/drivers')])
      setBuses(Array.isArray(busData) ? busData : [])
      setRoutes(Array.isArray(routeData) ? routeData : [])
      setDrivers(Array.isArray(driverData) ? driverData : [])
    } catch (err) { setError(err.message || 'Could not load the fleet data.') } finally { setLoading(false) }
  }
  useEffect(() => { loadData() }, [])

  const goTo = (nextView) => { setMessage(''); setError(''); setView(nextView) }
  const setPoint = (key, value) => setRouteForm((current) => key === 'name' ? { ...current, name: value } : ({ ...current, [key]: { ...current[key], ...value } }))
  const setStop = (index, value) => setStops((current) => current.map((stop, i) => i === index ? { ...stop, ...value } : stop))
  const moveStop = (from, to) => setStops((current) => { const next = [...current]; const [item] = next.splice(from, 1); next.splice(to, 0, item); return next })
  const selectBus = (busId) => { setSelectedBusId(busId); const bus = buses.find((item) => item.id === busId); setBusForm(bus ? { license_plate: bus.license_plate, capacity: String(bus.capacity), status: bus.status || 'ACTIVE', route_id: bus.route_id || '', driver_id: bus.driver_id || '' } : emptyBus) }
  const selectRoute = async (routeId) => {
    setSelectedRouteId(routeId); setStops([])
    const route = routes.find((item) => item.id === routeId)
    if (!route) { setRouteForm(emptyRoute); return }
    try {
      const routeStops = await api.get(`/api/v1/routes/${routeId}/stops`)
      const ordered = Array.isArray(routeStops) ? [...routeStops].sort((a, b) => a.sequence_order - b.sequence_order) : []
      setRouteForm({ name: route.name, start: ordered[0] ? pointFromStop(ordered[0]) : { ...emptyPoint, name: route.origin }, end: ordered.length > 1 ? pointFromStop(ordered.at(-1)) : { ...emptyPoint, name: route.destination } })
      setStops(ordered.length > 2 ? ordered.slice(1, -1).map(pointFromStop) : [])
    } catch (err) { setError(err.message || 'Could not load route stops.') }
  }
  const allRouteStops = () => [routeForm.start, ...stops, routeForm.end]
  const validatePoints = () => allRouteStops().every((stop) => stop.name.trim() && Number.isFinite(Number(stop.latitude)) && Number(stop.latitude) >= -90 && Number(stop.latitude) <= 90 && Number.isFinite(Number(stop.longitude)) && Number(stop.longitude) >= -180 && Number(stop.longitude) <= 180)

  const submitRoute = async (event) => {
    event.preventDefault(); setSaving(true); setError(''); setMessage('')
    if (!validatePoints()) { setSaving(false); setError('Every start, end, and stop point needs a name and valid latitude/longitude.'); return }
    try {
      const payload = { name: routeForm.name, origin: routeForm.start.name, destination: routeForm.end.name }
      let routeId = selectedRouteId
      if (view === 'createRoute') {
        const created = await api.post('/api/v1/routes', payload)
        routeId = created.route_id
        for (const [index, stop] of allRouteStops().entries()) await api.post(`/api/v1/routes/${routeId}/stops`, stopPayload(stop, index + 1))
        setRouteForm(emptyRoute); setStops([]); setMessage('Route and its stops were created successfully.')
      } else {
        await api.put(`/api/v1/routes/${routeId}`, payload)
        const savedIds = []
        for (const [index, stop] of allRouteStops().entries()) {
          if (stop.id) { await api.put(`/api/v1/routes/${routeId}/stops/${stop.id}`, stopPayload(stop, index + 1)); savedIds.push(stop.id) }
          else { const created = await api.post(`/api/v1/routes/${routeId}/stops`, stopPayload(stop, index + 1)); savedIds.push(created.stop_id) }
        }
        await api.put(`/api/v1/routes/${routeId}/stops/reorder`, { stop_ids: savedIds })
        setMessage('Route and stop order were updated successfully.')
        await selectRoute(routeId)
      }
      await loadData()
    } catch (err) { setError(err.message || 'Could not save route.') } finally { setSaving(false) }
  }
  const deleteStop = async (index) => {
    const stop = stops[index]
    if (!stop?.id) { setStops((current) => current.filter((_, i) => i !== index)); return }
    if (!window.confirm(`Remove ${stop.name}?`)) return
    setSaving(true); setError('')
    try { await api.del(`/api/v1/routes/${selectedRouteId}/stops/${stop.id}`); setStops((current) => current.filter((_, i) => i !== index)); setMessage('Stop removed.') } catch (err) { setError(err.message || 'Could not remove stop.') } finally { setSaving(false) }
  }
  const submitBus = async (event) => {
    event.preventDefault(); setSaving(true); setError(''); setMessage('')
    try {
      const payload = { ...busForm, capacity: Number(busForm.capacity) }
      if (view === 'createBus') { await api.post('/api/v1/fleet/buses', payload); setBusForm(emptyBus); setMessage('Bus and assignment created successfully.') }
      else { await api.put(`/api/v1/fleet/buses/${selectedBusId}`, payload); setMessage('Bus, route, and driver were updated successfully.') }
      await loadData()
    } catch (err) { setError(err.message || 'Could not save bus.') } finally { setSaving(false) }
  }
  const deleteSelected = async (kind) => {
    const id = kind === 'route' ? selectedRouteId : selectedBusId
    if (!id || !window.confirm(`Delete this ${kind}? This cannot be undone.`)) return
    setSaving(true); setError(''); setMessage('')
    try { await api.del(kind === 'route' ? `/api/v1/routes/${id}` : `/api/v1/fleet/buses/${id}`); kind === 'route' ? selectRoute('') : selectBus(''); setMessage(`${kind[0].toUpperCase()}${kind.slice(1)} deleted successfully.`); await loadData() } catch (err) { setError(err.message || `Could not delete ${kind}.`) } finally { setSaving(false) }
  }
  const filteredBuses = buses.filter((bus) => bus.license_plate.toLowerCase().includes(query.trim().toLowerCase()) && (!statusFilter || bus.status === statusFilter))
  const formMessage = <>{error && <p className="mb-3 text-sm text-[#991b1b]">{error}</p>}{message && <p className="mb-3 text-sm text-[#3f6212]">{message}</p>}</>
  const handleLogout = () => { logout(); navigate('/') }

  return <div className="flex min-h-screen bg-[#f5f5f4]">
    <aside className="w-[210px] flex-shrink-0 bg-white border-r border-[#e7e5e4] px-3 py-[18px] flex flex-col"><div className="flex items-center gap-2 text-sm font-bold mb-4 px-2"><i className="ti ti-bus text-[19px]" />Bus<span className="text-[#3f6212]">Yatri</span></div>{navItems.map((item) => <button key={item.key} type="button" onClick={() => goTo(item.key)} className={`w-full flex items-center gap-[10px] px-3 py-[10px] rounded-lg text-[13px] font-semibold text-left mb-[2px] ${view === item.key ? 'bg-[#ecfccb] text-[#3f6212]' : 'text-[#44403c] hover:bg-[#f5f5f4]'}`}><i className={`ti ${item.icon} text-[17px]`} />{item.label}</button>)}<button type="button" onClick={handleLogout} className="mt-auto w-full flex items-center gap-[10px] px-3 py-[10px] rounded-lg text-[13px] font-semibold text-left text-[#991b1b] hover:bg-[#fee2e2]"><i className="ti ti-logout text-[17px]" />Logout</button></aside>
    <main className="flex-1 px-[30px] py-6 overflow-x-hidden">{formMessage}
      {view === 'dashboard' && <Dashboard buses={filteredBuses} routes={routes} loading={loading} query={query} setQuery={setQuery} statusFilter={statusFilter} setStatusFilter={setStatusFilter} onEditBus={(id) => { selectBus(id); goTo('editBus') }} onEditRoute={(id) => { selectRoute(id); goTo('editRoute') }} />}
      {view === 'createRoute' && <form className={cardClass} onSubmit={submitRoute}><h1 className="font-bold mb-4">Create route</h1><RouteEditor form={routeForm} setPoint={setPoint} stops={stops} setStop={setStop} setStops={setStops} moveStop={moveStop} draggedStop={draggedStop} setDraggedStop={setDraggedStop} /><button disabled={saving} className="mt-5 bg-[#3f6212] text-white px-[18px] py-[10px] rounded-lg text-[13px] font-semibold">{saving ? 'Saving…' : 'Save route'}</button></form>}
      {view === 'editRoute' && <form className={cardClass} onSubmit={submitRoute}><h1 className="font-bold mb-4">Edit route</h1><RouteSelect routes={routes} value={selectedRouteId} onChange={(id) => selectRoute(id)} />{selectedRouteId && <><RouteEditor form={routeForm} setPoint={setPoint} stops={stops} setStop={setStop} setStops={setStops} moveStop={moveStop} draggedStop={draggedStop} setDraggedStop={setDraggedStop} onDeleteStop={deleteStop} /><div className="flex gap-3 mt-5"><button disabled={saving} className="bg-[#3f6212] text-white px-[18px] py-[10px] rounded-lg text-[13px] font-semibold">Save changes</button><button type="button" disabled={saving} onClick={() => deleteSelected('route')} className="bg-[#fee2e2] text-[#991b1b] px-[18px] py-[10px] rounded-lg text-[13px] font-semibold">Delete route</button></div></>}</form>}
      {view === 'createBus' && <form className={cardClass} onSubmit={submitBus}><h1 className="font-bold mb-4">Create bus</h1><BusFields value={busForm} onChange={setBusForm} routes={routes} drivers={drivers} includeStatus={false} /><button disabled={saving} className="mt-5 bg-[#3f6212] text-white px-[18px] py-[10px] rounded-lg text-[13px] font-semibold">{saving ? 'Saving…' : 'Save bus'}</button></form>}
      {view === 'editBus' && <form className={cardClass} onSubmit={submitBus}><h1 className="font-bold mb-4">Edit bus</h1><label className={fieldLabelClass}>SELECT BUS</label><select className={`${inputClass} mb-4`} value={selectedBusId} onChange={(e) => selectBus(e.target.value)}><option value="">Choose a bus</option>{buses.map((bus) => <option value={bus.id} key={bus.id}>{bus.license_plate}</option>)}</select>{selectedBusId && <><BusFields value={busForm} onChange={setBusForm} routes={routes} drivers={drivers} includeStatus /><div className="flex gap-3 mt-5"><button disabled={saving} className="bg-[#3f6212] text-white px-[18px] py-[10px] rounded-lg text-[13px] font-semibold">Save changes</button><button type="button" disabled={saving} onClick={() => deleteSelected('bus')} className="bg-[#fee2e2] text-[#991b1b] px-[18px] py-[10px] rounded-lg text-[13px] font-semibold">Delete bus</button></div></>}</form>}
      {view === 'track' && <TrackBuses buses={buses} routes={routes} onTrack={(bus) => navigate(`/track?busId=${bus.id}&routeId=${bus.route_id || ''}&from=${encodeURIComponent(routes.find((route) => route.id === bus.route_id)?.origin || 'Origin')}&to=${encodeURIComponent(routes.find((route) => route.id === bus.route_id)?.destination || 'Destination')}`)} />}
    </main>
  </div>
}

const pointFromStop = (stop) => ({ id: stop.id, name: stop.name, latitude: String(stop.latitude), longitude: String(stop.longitude) })
const stopPayload = (stop, sequence_order) => ({ name: stop.name.trim(), latitude: Number(stop.latitude), longitude: Number(stop.longitude), sequence_order })
const RouteSelect = ({ routes, value, onChange }) => <><label className={fieldLabelClass}>SELECT ROUTE</label><select className={`${inputClass} mb-4`} value={value} onChange={(e) => onChange(e.target.value)}><option value="">Choose a route</option>{routes.map((route) => <option value={route.id} key={route.id}>{route.name} · {route.origin} → {route.destination}</option>)}</select></>
const PointFields = ({ label, point, onChange }) => <div className="border border-[#e7e5e4] rounded-lg p-3"><p className="text-xs font-bold text-[#44403c] mb-3">{label}</p><div className="grid md:grid-cols-3 gap-[10px]"><Field label="POINT NAME" value={point.name} onChange={(value) => onChange({ name: value })} /><Field label="LATITUDE" type="number" step="any" min="-90" max="90" value={point.latitude} onChange={(value) => onChange({ latitude: value })} /><Field label="LONGITUDE" type="number" step="any" min="-180" max="180" value={point.longitude} onChange={(value) => onChange({ longitude: value })} /></div></div>
const RouteEditor = ({ form, setPoint, stops, setStop, setStops, moveStop, draggedStop, setDraggedStop, onDeleteStop }) => <><Field label="ROUTE NAME" value={form.name} onChange={(name) => setPoint('name', name)} routeName /><div className="grid gap-3 mt-3"><PointFields label="Start point" point={form.start} onChange={(value) => setPoint('start', value)} /><div><div className="flex justify-between items-center mb-2"><p className="text-xs font-bold text-[#44403c]">Intermediate stops</p><button type="button" onClick={() => setStops((current) => [...current, newStop()])} className="text-sm font-semibold text-[#3f6212]">+ Add stop</button></div><p className="text-xs text-[#78716c] mb-2">Drag stops to rearrange them. Start and end points remain fixed.</p>{stops.map((stop, index) => <div key={stop.id || `new-${index}`} draggable onDragStart={() => setDraggedStop(index)} onDragOver={(event) => event.preventDefault()} onDrop={() => { if (draggedStop !== null && draggedStop !== index) moveStop(draggedStop, index); setDraggedStop(null) }} className="mb-2 border border-[#e7e5e4] rounded-lg p-3 bg-white"><div className="flex justify-between gap-3"><p className="text-xs font-bold text-[#44403c]">↕ Stop {index + 1}</p><button type="button" onClick={() => onDeleteStop ? onDeleteStop(index) : setStops((current) => current.filter((_, i) => i !== index))} className="text-xs font-semibold text-[#991b1b]">Remove</button></div><div className="grid md:grid-cols-3 gap-[10px] mt-2"><Field label="STOP NAME" value={stop.name} onChange={(value) => setStop(index, { name: value })} /><Field label="LATITUDE" type="number" step="any" min="-90" max="90" value={stop.latitude} onChange={(value) => setStop(index, { latitude: value })} /><Field label="LONGITUDE" type="number" step="any" min="-180" max="180" value={stop.longitude} onChange={(value) => setStop(index, { longitude: value })} /></div></div>)}</div><PointFields label="End point" point={form.end} onChange={(value) => setPoint('end', value)} /></div></>
const Field = ({ label, value, onChange, routeName, ...props }) => <div className={routeName ? 'mb-3' : ''}><label className={fieldLabelClass}>{label}</label><input required maxLength="100" className={inputClass} value={value} onChange={(e) => onChange(e.target.value)} {...props} /></div>
const BusFields = ({ value, onChange, routes, drivers, includeStatus }) => (
  <div className="grid md:grid-cols-3 gap-[14px]">
    <Field label="BUS NUMBER" value={value.license_plate} onChange={(license_plate) => onChange({ ...value, license_plate })} />
    <Field label="CAPACITY" type="number" min="1" max="200" value={value.capacity} onChange={(capacity) => onChange({ ...value, capacity })} />
    <SelectField label="SELECT ROUTE" value={value.route_id} onChange={(route_id) => onChange({ ...value, route_id })} options={routes} placeholder="No route assignment" labelFor={(route) => `${route.name} · ${route.origin} → ${route.destination}`} />
    <SelectField label="SELECT DRIVER" value={value.driver_id} onChange={(driver_id) => onChange({ ...value, driver_id })} options={drivers} placeholder="No driver assignment" labelFor={(driver) => `${driver.name} · ${driver.email}`} />
    {includeStatus && <SelectField label="STATUS" value={value.status} onChange={(status) => onChange({ ...value, status })} options={statuses.map((status) => ({ id: status, name: status }))} labelFor={(item) => item.name} />}
  </div>
)
const SelectField = ({ label, value, onChange, options, placeholder, labelFor }) => <div><label className={fieldLabelClass}>{label}</label><select className={inputClass} value={value} onChange={(e) => onChange(e.target.value)}><option value="">{placeholder}</option>{options.map((option) => <option value={option.id} key={option.id}>{labelFor(option)}</option>)}</select></div>
const Dashboard = ({ buses, routes, loading, query, setQuery, statusFilter, setStatusFilter, onEditBus, onEditRoute }) => <><div className="grid grid-cols-2 md:grid-cols-4 gap-3 mb-[18px]">{[['Total buses', buses.length], ['Active now', buses.filter((bus) => bus.status === 'ACTIVE').length], ['Routes', routes.length], ['In maintenance', buses.filter((bus) => bus.status === 'MAINTENANCE').length]].map(([label, value]) => <div key={label} className="bg-white rounded-xl px-[18px] py-4 shadow-[0_1px_2px_rgba(0,0,0,0.05)]"><div className="text-[#78716c] text-sm">{label}</div><div className="text-[22px] font-bold mt-1">{loading ? '—' : value}</div></div>)}</div><div className={cardClass}><div className="flex flex-wrap gap-[14px]"><div className="flex-1 min-w-[150px]"><label className={fieldLabelClass}>SEARCH BUS ID</label><input className={inputClass} value={query} onChange={(e) => setQuery(e.target.value)} placeholder="e.g. UP70AB1234" /></div><div className="flex-1 min-w-[150px]"><label className={fieldLabelClass}>STATUS</label><select className={inputClass} value={statusFilter} onChange={(e) => setStatusFilter(e.target.value)}><option value="">All statuses</option>{statuses.map((status) => <option key={status}>{status}</option>)}</select></div></div></div><section className="bg-white rounded-xl shadow-[0_1px_2px_rgba(0,0,0,0.05)] overflow-hidden mt-[14px]"><div className="grid grid-cols-[1.2fr_1fr_1fr_1fr_0.5fr] bg-[#78716c] text-white text-[11px] font-bold px-5 py-[11px]"><div>BUS ID</div><div>ROUTE</div><div>DRIVER</div><div>STATUS</div><div>ACTION</div></div>{buses.map((bus) => <div key={bus.id} className="grid grid-cols-[1.2fr_1fr_1fr_1fr_0.5fr] px-5 py-4 border-b border-[#f5f5f4] items-center text-[13px]"><div className="font-bold">{bus.license_plate}</div><div>{bus.route_name || 'Unassigned'}</div><div>{bus.driver_name || 'Unassigned'}</div><div>{bus.status}</div><button type="button" onClick={() => onEditBus(bus.id)} className="text-left text-[#3f6212] font-semibold">Edit</button></div>)}{!loading && buses.length === 0 && <p className="p-5 text-sm text-[#78716c]">No buses found.</p>}</section><section className={`${cardClass} mt-[14px]`}><h2 className="font-bold text-sm mb-3">Routes</h2>{routes.map((route) => <div key={route.id} className="flex justify-between gap-3 py-3 border-t border-[#f5f5f4] text-sm"><span><b>{route.name}</b> · {route.origin} → {route.destination}</span><button type="button" onClick={() => onEditRoute(route.id)} className="text-[#3f6212] font-semibold">Edit</button></div>)}</section></>
const TrackBuses = ({ buses, routes, onTrack }) => <section className={cardClass}><h1 className="font-bold mb-1">Track buses</h1><p className="text-sm text-[#78716c] mb-4">Open the same live tracking page passengers use.</p>{buses.map((bus) => <div key={bus.id} className="flex justify-between gap-4 items-center py-3 border-t border-[#f5f5f4]"><div><p className="font-semibold text-sm">{bus.license_plate}</p><p className="text-xs text-[#78716c]">{bus.route_name || routes.find((route) => route.id === bus.route_id)?.name || 'No route assigned'} · {bus.driver_name || 'No driver assigned'}</p></div><button type="button" onClick={() => onTrack(bus)} className="text-sm font-semibold text-[#3f6212]">Track</button></div>)}{buses.length === 0 && <p className="text-sm text-[#78716c]">No buses have been created yet.</p>}</section>

export default Admin
