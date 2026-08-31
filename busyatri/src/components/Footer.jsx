import React from 'react'
import { BusFront } from 'lucide-react'

const Footer = () => {
  return (
    <div className='flex items-center justify-end gap-2 bg-gray-900 px-6 py-3 text-white'>
        <BusFront size={20} />
        <span className='font-semibold'>BusYatri</span>
    </div>
  )
}

export default Footer