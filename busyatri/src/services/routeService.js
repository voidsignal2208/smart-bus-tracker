import { api } from './api'

export const routeService = {
  getRoutes: () => api.get('/api/v1/routes'),
  createRoute: (name, origin, destination) =>
    api.post('/api/v1/routes', { name, origin, destination }),
  getRouteStops: (routeId) => api.get(`/api/v1/routes/${routeId}/stops`),
  addStop: (routeId, name, latitude, longitude, sequence_order) =>
    api.post(`/api/v1/routes/${routeId}/stops`, { name, latitude, longitude, sequence_order }),
  getRouteBuses: (routeId, date) => api.get(`/api/v1/routes/${routeId}/buses${date ? `?date=${date}` : ''}`),
  assignBus: (routeId, busId, status) =>
    api.post(`/api/v1/routes/${routeId}/buses`, { bus_id: busId, status }),
}
