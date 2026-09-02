// Thin fetch wrapper around the Drogon backend's REST API.
//
// Left as relative paths (e.g. "/api/v1/...") on purpose: in dev, Vite's
// proxy (vite.config.js) forwards these to the backend so the browser
// never makes a cross-origin request (the backend has no CORS headers
// configured). In production, this assumes the frontend build and the
// API are served from the same origin (e.g. via the Nginx setup the
// project's README describes) - set VITE_API_BASE_URL if that's not the
// case, but then the backend will also need CORS support added.
const API_BASE = (import.meta.env.VITE_API_BASE_URL || '').replace(/\/$/, '')

const TOKEN_STORAGE_KEY = 'busyatri_token'

export function getToken() {
  return localStorage.getItem(TOKEN_STORAGE_KEY)
}

export function setToken(token) {
  if (token) {
    localStorage.setItem(TOKEN_STORAGE_KEY, token)
  } else {
    localStorage.removeItem(TOKEN_STORAGE_KEY)
  }
}

// Thrown for any non-2xx response. Carries the HTTP status and whatever
// JSON body the backend sent (usually `{ error: "..." }`), so callers can
// show a real message instead of a generic one.
export class ApiError extends Error {
  constructor(status, body) {
    super((body && body.error) || `Request failed with status ${status}`)
    this.status = status
    this.body = body
  }
}

async function request(path, { method = 'GET', body, auth = false, params } = {}) {
  let url = `${API_BASE}${path}`

  if (params) {
    const search = new URLSearchParams(
      Object.entries(params).filter(([, v]) => v !== undefined && v !== null && v !== '')
    ).toString()
    if (search) url += `?${search}`
  }

  const headers = {}
  if (body !== undefined) headers['Content-Type'] = 'application/json'
  if (auth) {
    const token = getToken()
    if (token) headers['Authorization'] = `Bearer ${token}`
  }

  let res
  try {
    res = await fetch(url, {
      method,
      headers,
      body: body !== undefined ? JSON.stringify(body) : undefined,
    })
  } catch (networkErr) {
    // fetch() itself throws on network failure / server unreachable /
    // CORS rejection - normalize that into the same ApiError shape.
    throw new ApiError(0, { error: 'Could not reach the server. Is the backend running?' })
  }

  const isJson = res.headers.get('content-type')?.includes('application/json')
  const data = isJson ? await res.json().catch(() => null) : null

  if (!res.ok) {
    throw new ApiError(res.status, data)
  }

  return data
}

// ---------------------------------------------------------------------
// Auth
// ---------------------------------------------------------------------
export function registerUser({ name, email, password }) {
  return request('/api/v1/auth/register', { method: 'POST', body: { name, email, password } })
}

export function loginUser({ email, password }) {
  return request('/api/v1/auth/login', { method: 'POST', body: { email, password } })
}

// ---------------------------------------------------------------------
// Fleet
// ---------------------------------------------------------------------
export function getBuses() {
  return request('/api/v1/fleet/buses', { auth: true })
}

export function createBus({ license_plate, capacity }) {
  return request('/api/v1/fleet/buses', {
    method: 'POST',
    auth: true,
    body: { license_plate, capacity },
  })
}

// ---------------------------------------------------------------------
// Routes & stops
// ---------------------------------------------------------------------
export function getRoutes() {
  return request('/api/v1/routes', { auth: true })
}

export function createRoute({ name, origin, destination }) {
  return request('/api/v1/routes', {
    method: 'POST',
    auth: true,
    body: { name, origin, destination },
  })
}

export function getRouteStops(routeId) {
  return request(`/api/v1/routes/${routeId}/stops`, { auth: true })
}

// Buses currently assigned to a route (bus_assignments join) - lets a
// route (or a from/to search match) resolve straight to the buses
// running it.
export function getRouteBuses(routeId) {
  return request(`/api/v1/routes/${routeId}/buses`, { auth: true })
}

export function addStop(routeId, { name, latitude, longitude, sequence_order }) {
  return request(`/api/v1/routes/${routeId}/stops`, {
    method: 'POST',
    auth: true,
    body: { name, latitude, longitude, sequence_order },
  })
}

// ---------------------------------------------------------------------
// Live tracking (HTTP side - see also lib/trackingSocket.js for the WS
// side, which is what the Track page actually uses for live updates)
// ---------------------------------------------------------------------
export function getLatestLocation(busId) {
  return request(`/api/v1/tracking/buses/${busId}/location`, { auth: true })
}

export function postLocation(busId, { latitude, longitude, speed_kmh }) {
  return request(`/api/v1/tracking/buses/${busId}/location`, {
    method: 'POST',
    auth: true,
    body: { latitude, longitude, speed_kmh },
  })
}

export function getLocationHistory(busId, limit = 50) {
  return request(`/api/v1/tracking/buses/${busId}/history`, { auth: true, params: { limit } })
}

// ---------------------------------------------------------------------
// Maps (server-side Google Directions proxy - keeps the server API key
// off the client entirely)
// ---------------------------------------------------------------------
// `waypoints` is an optional ordered array of { lat, lng } - the route's
// stops, in sequence_order - so the returned polyline is the actual
// road-following path through every stop (via Google Directions), not
// just a straight line between origin and destination.
export function getRoutePolyline({ originLat, originLng, destLat, destLng, waypoints }) {
  const waypointsParam =
    Array.isArray(waypoints) && waypoints.length > 0
      ? waypoints.map((w) => `${w.lat},${w.lng}`).join('|')
      : undefined

  return request('/api/v1/maps/route', {
    auth: true,
    params: {
      origin_lat: originLat,
      origin_lng: originLng,
      dest_lat: destLat,
      dest_lng: destLng,
      waypoints: waypointsParam,
    },
  })
}
