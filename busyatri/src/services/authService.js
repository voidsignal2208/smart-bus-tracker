import { api } from './api'

export const authService = {
  login: (email, password) =>
    api.post('/api/v1/auth/login', { email, password }),
  register: (name, email, password, role, phone) =>
    api.post('/api/v1/auth/register', { name, email, password, role, phone }),
}
