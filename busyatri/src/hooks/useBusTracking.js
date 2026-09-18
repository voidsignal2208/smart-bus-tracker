import { useState, useEffect, useRef, useCallback } from 'react'
import { getToken } from '../services/api'

const MAX_RECONNECT_DELAY = 30000
const INITIAL_RECONNECT_DELAY = 1000

export function useBusTracking(busId) {
  const [location, setLocation] = useState(null)
  const [connected, setConnected] = useState(false)
  const [error, setError] = useState(null)
  const wsRef = useRef(null)
  const reconnectTimeoutRef = useRef(null)
  const reconnectDelayRef = useRef(INITIAL_RECONNECT_DELAY)

  const connect = useCallback(() => {
    if (!busId) return

    const token = getToken()
    const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:'
    const wsUrl = token
      ? `${protocol}//${window.location.host}/ws/tracking?token=${token}`
      : `${protocol}//${window.location.host}/ws/tracking`

    try {
      const ws = new WebSocket(wsUrl)
      wsRef.current = ws

      ws.onopen = () => {
        setConnected(true)
        setError(null)
        reconnectDelayRef.current = INITIAL_RECONNECT_DELAY

        // Subscribe to bus updates
        ws.send(JSON.stringify({
          action: 'subscribe',
          bus_id: busId,
        }))
      }

      ws.onmessage = (event) => {
        try {
          const data = JSON.parse(event.data)
          // Ignore ack messages
          if (data.message === 'subscribed') return
          if (data.error) {
            setError(data.error)
            return
          }
          // Update location state with broadcast data
          if (data.latitude !== undefined && data.longitude !== undefined) {
            setLocation({
              lat: parseFloat(data.latitude),
              lng: parseFloat(data.longitude),
              speed: data.speed_kmh ? parseFloat(data.speed_kmh) : 0,
              timestamp: data.timestamp,
              busId: data.bus_id,
            })
          }
        } catch (e) {
          console.error('Failed to parse WS message:', e)
        }
      }

      ws.onerror = () => {
        setError('WebSocket connection error')
      }

      ws.onclose = () => {
        setConnected(false)
        wsRef.current = null

        // Auto-reconnect with exponential backoff
        const delay = reconnectDelayRef.current
        reconnectDelayRef.current = Math.min(delay * 2, MAX_RECONNECT_DELAY)
        reconnectTimeoutRef.current = setTimeout(connect, delay)
      }
    } catch (e) {
      setError('Failed to create WebSocket connection')
    }
  }, [busId])

  useEffect(() => {
    connect()

    return () => {
      if (reconnectTimeoutRef.current) {
        clearTimeout(reconnectTimeoutRef.current)
      }
      if (wsRef.current) {
        wsRef.current.close()
        wsRef.current = null
      }
    }
  }, [connect])

  return { location, connected, error }
}
