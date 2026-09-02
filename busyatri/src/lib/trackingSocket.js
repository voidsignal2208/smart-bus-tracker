import { getToken } from './api'

// Builds the WebSocket URL for the backend's /ws/tracking endpoint.
//
// Deliberately built from window.location (not VITE_API_BASE_URL): in dev
// this hits the current origin (localhost:5173) and Vite's proxy
// (vite.config.js) forwards the upgrade request to the backend on 8080.
// In production, this assumes frontend + backend share an origin, same
// as the REST calls in lib/api.js.
function trackingSocketUrl(token) {
  const scheme = window.location.protocol === 'https:' ? 'wss' : 'ws'
  return `${scheme}://${window.location.host}/ws/tracking?token=${encodeURIComponent(token)}`
}

// Opens a connection to the live tracking channel and subscribes to a
// single bus. Calls onLocation(payload) every time the backend broadcasts
// a new reading for that bus (see backend/src/controllers/
// TrackingWebSocketCtrl.cc for the exact message shapes).
//
// Returns a cleanup function - call it (e.g. from a useEffect return) to
// unsubscribe and close the socket.
export function subscribeToBusLocation(busId, { onLocation, onError, onOpen } = {}) {
  const token = getToken()
  if (!token) {
    onError?.(new Error('Not logged in'))
    return () => {}
  }

  const socket = new WebSocket(trackingSocketUrl(token))
  let closedByUs = false

  socket.onopen = () => {
    socket.send(JSON.stringify({ action: 'subscribe', bus_id: busId }))
    onOpen?.()
  }

  socket.onmessage = (event) => {
    let payload
    try {
      payload = JSON.parse(event.data)
    } catch {
      return
    }

    if (payload.error) {
      onError?.(new Error(payload.error))
      return
    }

    // Ignore the {"message":"subscribed",...} ack - only forward actual
    // location updates (which always carry latitude/longitude).
    if (payload.latitude !== undefined && payload.longitude !== undefined) {
      onLocation?.(payload)
    }
  }

  socket.onerror = () => {
    if (!closedByUs) onError?.(new Error('Tracking connection error'))
  }

  return () => {
    closedByUs = true
    socket.close()
  }
}

// Pushes a single location reading over the WebSocket. Used by the
// Driver page. Requires a DRIVER/CONDUCTOR/ADMIN account - the backend
// enforces that (see TrackingWebSocketCtrl::handleNewMessage), this is
// just a thin client-side wrapper.
export function openDriverPushSocket({ onAck, onError, onOpen } = {}) {
  const token = getToken()
  if (!token) {
    onError?.(new Error('Not logged in'))
    return null
  }

  const socket = new WebSocket(trackingSocketUrl(token))

  socket.onopen = () => onOpen?.()

  socket.onmessage = (event) => {
    let payload
    try {
      payload = JSON.parse(event.data)
    } catch {
      return
    }
    if (payload.error) {
      onError?.(new Error(payload.error))
      return
    }
    onAck?.(payload)
  }

  socket.onerror = () => onError?.(new Error('Tracking connection error'))

  return {
    pushLocation(busId, { latitude, longitude, speed_kmh }) {
      if (socket.readyState !== WebSocket.OPEN) return
      socket.send(
        JSON.stringify({
          action: 'push_location',
          bus_id: busId,
          latitude,
          longitude,
          ...(speed_kmh !== undefined ? { speed_kmh } : {}),
        })
      )
    },
    close() {
      socket.close()
    },
  }
}
