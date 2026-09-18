import React, { useState } from 'react'
import { Link, useLocation, useNavigate } from 'react-router-dom'
import Navbar from '../components/Navbar'
import { useAuth, ROLES } from '../context/AuthContext'
import { AlertCircle, CheckCircle2, Loader2, ShieldCheck, BusFront } from 'lucide-react'

const Login = () => {
  const navigate = useNavigate()
  const location = useLocation()
  const { login, logout } = useAuth()
  const [selectedRole, setSelectedRole] = useState(ROLES.DRIVER)
  const [email, setEmail] = useState(location.state?.email || '')
  const [password, setPassword] = useState('')
  const [error, setError] = useState(null)
  const [loading, setLoading] = useState(false)
  const justRegistered = Boolean(location.state?.registered)
  const selectedRoleName = selectedRole === ROLES.ADMIN ? 'Admin' : 'Driver'

  const handleSubmit = async (event) => {
    event.preventDefault()
    setError(null)
    setLoading(true)
    try {
      const user = await login(email, password)
      if (user.roleId !== selectedRole) {
        logout()
        throw new Error(`These credentials do not belong to a ${selectedRoleName} account.`)
      }
      navigate(selectedRole === ROLES.ADMIN ? '/admin' : '/driver')
    } catch (err) {
      setError(err.message || 'Unable to sign in.')
    } finally {
      setLoading(false)
    }
  }

  return (
    <div className='min-h-screen bg-amber-50 flex flex-col'>
      <Navbar />
      <main className='flex-1 flex items-center justify-center px-4 pt-16 pb-8'>
        <section className='w-full max-w-sm bg-white rounded-xl shadow-sm border border-gray-200 p-6'>
          <h1 className='text-xl font-bold text-gray-900'>Staff login</h1>
          <p className='mt-1 text-sm text-gray-500'>Choose your account type, then enter your existing credentials.</p>
          {justRegistered && <div className='mt-4 flex items-center gap-2 text-sm text-green-700 bg-green-50 border border-green-200 rounded-lg px-3 py-2'><CheckCircle2 size={16} />Account created. Sign in with your new credentials.</div>}
          <form onSubmit={handleSubmit} className='mt-6 space-y-4'>
            <fieldset>
              <legend className='text-xs font-semibold text-gray-500'>LOGIN AS</legend>
              <div className='mt-2 grid grid-cols-2 gap-3'>
                <button type='button' onClick={() => setSelectedRole(ROLES.DRIVER)} className={`rounded-lg border p-3 text-left text-sm font-semibold ${selectedRole === ROLES.DRIVER ? 'border-lime-800 bg-lime-50 text-lime-900 ring-1 ring-lime-800' : 'border-gray-300 text-gray-700 hover:bg-gray-50'}`}><BusFront size={18} className='mb-2' />Driver</button>
                <button type='button' onClick={() => setSelectedRole(ROLES.ADMIN)} className={`rounded-lg border p-3 text-left text-sm font-semibold ${selectedRole === ROLES.ADMIN ? 'border-lime-800 bg-lime-50 text-lime-900 ring-1 ring-lime-800' : 'border-gray-300 text-gray-700 hover:bg-gray-50'}`}><ShieldCheck size={18} className='mb-2' />Admin</button>
              </div>
            </fieldset>
            <div>
              <label htmlFor='email' className='text-xs font-semibold text-gray-500'>USERNAME / EMAIL</label>
              <input id='email' type='email' autoComplete='username' required value={email} onChange={(event) => setEmail(event.target.value)} className='mt-1 w-full border border-gray-300 rounded-lg px-3 py-2 text-sm focus:outline-none focus:ring-2 focus:ring-lime-700' />
            </div>
            <div>
              <label htmlFor='password' className='text-xs font-semibold text-gray-500'>PASSWORD</label>
              <input id='password' type='password' autoComplete='current-password' required value={password} onChange={(event) => setPassword(event.target.value)} className='mt-1 w-full border border-gray-300 rounded-lg px-3 py-2 text-sm focus:outline-none focus:ring-2 focus:ring-lime-700' />
            </div>
            {error && <div className='flex items-center gap-2 text-sm text-red-600 bg-red-50 border border-red-200 rounded-lg px-3 py-2'><AlertCircle size={16} />{error}</div>}
            <button type='submit' disabled={loading} className='w-full flex items-center justify-center gap-2 bg-lime-800 text-white rounded-lg py-2 text-sm font-semibold hover:bg-lime-900 disabled:opacity-60'>{loading && <Loader2 size={16} className='animate-spin' />}Sign in as {selectedRoleName}</button>
          </form>
          <p className='mt-4 text-center text-sm text-gray-500'>
            New driver or admin? <Link to='/register' className='font-semibold text-lime-800 hover:underline'>Create an account</Link>
          </p>
        </section>
      </main>
    </div>
  )
}

export default Login
