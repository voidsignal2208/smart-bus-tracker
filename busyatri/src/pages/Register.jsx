import React, { useState } from 'react'
import { Link, useNavigate } from 'react-router-dom'
import Navbar from '../components/Navbar'
import { ROLES } from '../context/AuthContext'
import { authService } from '../services/authService'
import { AlertCircle, Loader2, ShieldCheck, BusFront } from 'lucide-react'

const Register = () => {
  const navigate = useNavigate()
  const [selectedRole, setSelectedRole] = useState(ROLES.DRIVER)
  const [name, setName] = useState('')
  const [email, setEmail] = useState('')
  const [phone, setPhone] = useState('')
  const [password, setPassword] = useState('')
  const [confirmPassword, setConfirmPassword] = useState('')
  const [error, setError] = useState(null)
  const [loading, setLoading] = useState(false)
  const selectedRoleName = selectedRole === ROLES.ADMIN ? 'Admin' : 'Driver'

  const handleSubmit = async (event) => {
    event.preventDefault()
    setError(null)

    if (password !== confirmPassword) {
      setError('Passwords do not match.')
      return
    }
    if (password.length < 8) {
      setError('Password must be at least 8 characters long.')
      return
    }

    setLoading(true)
    try {
      const role = selectedRole === ROLES.ADMIN ? 'ADMIN' : 'DRIVER'
      await authService.register(name, email, password, role, phone)
      navigate('/login', { state: { registered: true, email } })
    } catch (err) {
      setError(err.message || 'Unable to create account.')
    } finally {
      setLoading(false)
    }
  }

  return (
    <div className='min-h-screen bg-amber-50 flex flex-col'>
      <Navbar />
      <main className='flex-1 flex items-center justify-center px-4 pt-16 pb-8'>
        <section className='w-full max-w-sm bg-white rounded-xl shadow-sm border border-gray-200 p-6'>
          <h1 className='text-xl font-bold text-gray-900'>Staff registration</h1>
          <p className='mt-1 text-sm text-gray-500'>Create your account, then sign in with the same credentials.</p>
          <form onSubmit={handleSubmit} className='mt-6 space-y-4'>
            <fieldset>
              <legend className='text-xs font-semibold text-gray-500'>REGISTER AS</legend>
              <div className='mt-2 grid grid-cols-2 gap-3'>
                <button type='button' onClick={() => setSelectedRole(ROLES.DRIVER)} className={`rounded-lg border p-3 text-left text-sm font-semibold ${selectedRole === ROLES.DRIVER ? 'border-lime-800 bg-lime-50 text-lime-900 ring-1 ring-lime-800' : 'border-gray-300 text-gray-700 hover:bg-gray-50'}`}><BusFront size={18} className='mb-2' />Driver</button>
                <button type='button' onClick={() => setSelectedRole(ROLES.ADMIN)} className={`rounded-lg border p-3 text-left text-sm font-semibold ${selectedRole === ROLES.ADMIN ? 'border-lime-800 bg-lime-50 text-lime-900 ring-1 ring-lime-800' : 'border-gray-300 text-gray-700 hover:bg-gray-50'}`}><ShieldCheck size={18} className='mb-2' />Admin</button>
              </div>
            </fieldset>
            <div>
              <label htmlFor='name' className='text-xs font-semibold text-gray-500'>FULL NAME</label>
              <input id='name' type='text' autoComplete='name' required value={name} onChange={(event) => setName(event.target.value)} className='mt-1 w-full border border-gray-300 rounded-lg px-3 py-2 text-sm focus:outline-none focus:ring-2 focus:ring-lime-700' />
            </div>
            <div>
              <label htmlFor='email' className='text-xs font-semibold text-gray-500'>EMAIL</label>
              <input id='email' type='email' autoComplete='username' required value={email} onChange={(event) => setEmail(event.target.value)} className='mt-1 w-full border border-gray-300 rounded-lg px-3 py-2 text-sm focus:outline-none focus:ring-2 focus:ring-lime-700' />
            </div>
            <div>
              <label htmlFor='phone' className='text-xs font-semibold text-gray-500'>PHONE (OPTIONAL)</label>
              <input id='phone' type='tel' autoComplete='tel' value={phone} onChange={(event) => setPhone(event.target.value)} className='mt-1 w-full border border-gray-300 rounded-lg px-3 py-2 text-sm focus:outline-none focus:ring-2 focus:ring-lime-700' />
            </div>
            <div>
              <label htmlFor='password' className='text-xs font-semibold text-gray-500'>PASSWORD</label>
              <input id='password' type='password' autoComplete='new-password' required minLength={8} value={password} onChange={(event) => setPassword(event.target.value)} className='mt-1 w-full border border-gray-300 rounded-lg px-3 py-2 text-sm focus:outline-none focus:ring-2 focus:ring-lime-700' />
            </div>
            <div>
              <label htmlFor='confirmPassword' className='text-xs font-semibold text-gray-500'>CONFIRM PASSWORD</label>
              <input id='confirmPassword' type='password' autoComplete='new-password' required minLength={8} value={confirmPassword} onChange={(event) => setConfirmPassword(event.target.value)} className='mt-1 w-full border border-gray-300 rounded-lg px-3 py-2 text-sm focus:outline-none focus:ring-2 focus:ring-lime-700' />
            </div>
            {error && <div className='flex items-center gap-2 text-sm text-red-600 bg-red-50 border border-red-200 rounded-lg px-3 py-2'><AlertCircle size={16} />{error}</div>}
            <button type='submit' disabled={loading} className='w-full flex items-center justify-center gap-2 bg-lime-800 text-white rounded-lg py-2 text-sm font-semibold hover:bg-lime-900 disabled:opacity-60'>{loading && <Loader2 size={16} className='animate-spin' />}Create {selectedRoleName} account</button>
          </form>
          <p className='mt-4 text-center text-sm text-gray-500'>
            Already have an account? <Link to='/login' className='font-semibold text-lime-800 hover:underline'>Sign in</Link>
          </p>
        </section>
      </main>
    </div>
  )
}

export default Register
