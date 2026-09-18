import { api } from './api'

export const fleetService = {
  getBuses: () => api.get('/api/v1/fleet/buses'),
  createBus: (license_plate, capacity) =>
    api.post('/api/v1/fleet/buses', { license_plate, capacity }),
  submitFeedback: (busId, data) =>
    api.post(`/api/v1/fleet/buses/${busId}/feedback`, data),
  getBusFeedback: (busId) =>
    api.get(`/api/v1/fleet/buses/${busId}/feedback`),
}

