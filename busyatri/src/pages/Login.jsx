import React, { useState } from 'react'
import { useNavigate, useLocation } from 'react-router-dom'
import { UserKey, Mail, Lock, User } from 'lucide-react'
import Navbar from '../components/Navbar'
import Footer from '../components/Footer'
import { useAuth } from '../context/AuthContext'
import { ApiError } from '../lib/api'
import { roleHomePath } from '../lib/roles'

const Login = () => {
  const [mode, setMode] = useState('login') // 'login' | 'register'
  const [name, setName] = useState('')
  const [email, setEmail] = useState('')
  const [password, setPassword] = useState('')
  const [error, setError] = useState('')
  const [submitting, setSubmitting] = useState(false)

  const { login, register } = useAuth()
  const navigate = useNavigate()
  const location = useLocation()

  const redirectAfterAuth = (loggedInUser) => {
    // If the user was bounced here by ProtectedRoute from some other
    // page, send them back there; otherwise send them to their role's
    // home page (passenger search, driver console, or admin dashboard).
    const from = location.state?.from
    navigate(from || roleHomePath(loggedInUser.roleId), { replace: true })
  }

  const handleSubmit = async (e) => {
    e.preventDefault()
    setError('')
    setSubmitting(true)
    try {
      const loggedInUser =
        mode === 'login' ? await login(email, password) : await register(name, email, password)
      redirectAfterAuth(loggedInUser)
    } catch (err) {
      if (err instanceof ApiError) {
        setError(err.message)
      } else {
        setError('Something went wrong. Please try again.')
      }
    } finally {
      setSubmitting(false)
    }
  }

  return (
    <div className='min-h-screen bg-amber-50 flex flex-col'>
      <Navbar />

      <div className='flex-1 flex items-center justify-center px-5 pt-20 pb-10'>
        <form
          onSubmit={handleSubmit}
          className='w-full max-w-sm rounded-xl bg-white p-6 shadow-lg'
        >
          <div className='mb-5 flex items-center gap-2'>
            <UserKey size={22} className='text-lime-800' />
            <h2 className='text-xl font-bold'>
              {mode === 'login' ? 'Welcome back' : 'Create your account'}
            </h2>
          </div>

          {mode === 'register' && (
            <>
              <label className='mb-1 block text-xs'>NAME</label>
              <div className='mb-3 flex items-center gap-2 rounded border border-gray-300 px-3 py-3'>
                <User size={20} />
                <input
                  type='text'
                  required
                  placeholder='Your name'
                  value={name}
                  onChange={(e) => setName(e.target.value)}
                  className='w-full outline-none'
                />
              </div>
            </>
          )}

          <label className='mb-1 block text-xs'>EMAIL</label>
          <div className='mb-3 flex items-center gap-2 rounded border border-gray-300 px-3 py-3'>
            <Mail size={20} />
            <input
              type='email'
              required
              placeholder='you@example.com'
              value={email}
              onChange={(e) => setEmail(e.target.value)}
              className='w-full outline-none'
            />
          </div>

          <label className='mb-1 block text-xs'>PASSWORD</label>
          <div className='mb-2 flex items-center gap-2 rounded border border-gray-300 px-3 py-3'>
            <Lock size={20} />
            <input
              type='password'
              required
              minLength={8}
              placeholder='At least 8 characters'
              value={password}
              onChange={(e) => setPassword(e.target.value)}
              className='w-full outline-none'
            />
          </div>

          {mode === 'register' && (
            <p className='mb-3 text-xs text-gray-500'>
              New accounts always start as a Passenger. Ask an administrator to upgrade
              a Driver/Conductor/Admin account.
            </p>
          )}

          {error && <p className='mb-3 text-sm text-red-600'>{error}</p>}

          <button
            type='submit'
            disabled={submitting}
            className='mt-2 flex w-full items-center justify-center gap-2 rounded bg-lime-800 py-3 font-bold text-white disabled:opacity-60'
          >
            {submitting ? 'Please wait...' : mode === 'login' ? 'Log in' : 'Sign up'}
          </button>

          <p className='mt-4 text-center text-sm text-gray-600'>
            {mode === 'login' ? (
              <>
                Don't have an account?{' '}
                <button
                  type='button'
                  className='font-semibold text-lime-800 hover:underline'
                  onClick={() => {
                    setMode('register')
                    setError('')
                  }}
                >
                  Sign up
                </button>
              </>
            ) : (
              <>
                Already have an account?{' '}
                <button
                  type='button'
                  className='font-semibold text-lime-800 hover:underline'
                  onClick={() => {
                    setMode('login')
                    setError('')
                  }}
                >
                  Log in
                </button>
              </>
            )}
          </p>
        </form>
      </div>

      <Footer />
    </div>
  )
}

export default Login
