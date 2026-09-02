// Minimal, dependency-free JWT payload reader.
//
// IMPORTANT: this does NOT verify the token's signature - it just reads
// the claims out of it so the UI can show the right role-based nav/routes.
// The backend independently verifies the signature on every request
// (JwtAuthFilter), so a tampered token simply gets rejected with a 401
// there. Never trust this decoded payload for anything security-critical
// on its own.
export function decodeJwt(token) {
  if (!token || typeof token !== 'string') return null

  const parts = token.split('.')
  if (parts.length !== 3) return null

  try {
    const payloadB64 = parts[1].replace(/-/g, '+').replace(/_/g, '/')
    const padded = payloadB64.padEnd(payloadB64.length + ((4 - (payloadB64.length % 4)) % 4), '=')
    const json = decodeURIComponent(
      atob(padded)
        .split('')
        .map((c) => '%' + c.charCodeAt(0).toString(16).padStart(2, '0'))
        .join('')
    )
    return JSON.parse(json)
  } catch {
    return null
  }
}

export function isTokenExpired(token) {
  const payload = decodeJwt(token)
  if (!payload || !payload.exp) return true
  // exp is seconds since epoch (standard JWT claim); Date.now() is ms.
  return Date.now() >= payload.exp * 1000
}
