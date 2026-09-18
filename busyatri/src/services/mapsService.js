import { api } from './api'

export const mapsService = {
  getRoutePolyline: (origin_lat, origin_lng, dest_lat, dest_lng) =>
    api.get(
      `/api/v1/maps/route?origin_lat=${origin_lat}&origin_lng=${origin_lng}&dest_lat=${dest_lat}&dest_lng=${dest_lng}`
    ),
}
