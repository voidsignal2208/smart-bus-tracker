import { api } from './api'

export const trackingService = {
  getLatestLocation: (busId) =>
    api.get(`/api/v1/tracking/buses/${busId}/location`),
  getLocationHistory: (busId, limit = 50) =>
    api.get(`/api/v1/tracking/buses/${busId}/history?limit=${limit}`),
  pushLocation: (busId, latitude, longitude, speed_kmh) =>
    api.post(`/api/v1/tracking/buses/${busId}/location`, { latitude, longitude, speed_kmh }),
}
