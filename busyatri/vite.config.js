import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'
import tailwindcss from '@tailwindcss/vite'

// https://vite.dev/config/
export default defineConfig({
  plugins: [react(), tailwindcss()],
  server: {
    // NOTE: this used to be 8080, the same port the backend listens on
    // (backend/.env APP_PORT). Moved to Vite's default so the two can run
    // side by side without a collision.
    port: 5173,
    proxy: {
      // Forwards browser requests for /api/* straight to the Drogon
      // backend during `npm run dev`, so the browser only ever talks to
      // one origin and we don't need to add CORS headers to the C++
      // server. Change the target if your backend runs elsewhere.
      '/api': {
        target: 'http://localhost:8080',
        changeOrigin: true,
      },
      // Same idea for the live-tracking WebSocket endpoint.
      '/ws': {
        target: 'ws://localhost:8080',
        ws: true,
      },
    },
  },
})
