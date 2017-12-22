/*
Author: Cao Thanh Tung, Ashwin Nanjappa
Date:   05-Aug-2014

===============================================================================

Copyright (c) 2011, School of Computing, National University of Singapore. 
All rights reserved.

Project homepage: http://www.comp.nus.edu.sg/~tants/gdel3d.html

If you use gDel3D and you like it or have comments on its usefulness etc., we 
would love to hear from you at <tants@comp.nus.edu.sg>. You may share with us
your experience and any possibilities that we may improve the work/code.

===============================================================================

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of
conditions and the following disclaimer. Redistributions in binary form must reproduce
the above copyright notice, this list of conditions and the following disclaimer
in the documentation and/or other materials provided with the distribution. 

Neither the name of the National University of University nor the names of its contributors
may be used to endorse or promote products derived from this software without specific
prior written permission from the National University of Singapore. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE IMPLIED WARRANTIES 
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
TO, PROCUREMENT OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
DAMAGE.
*/

#ifndef _DEV_VECTOR_H
#define _DEV_VECTOR_H

#include "basic_defs.h"

////////////////////////////////////////////////////////////////// DevVector //
template< typename T > 
class DevVector
{
public: 
    // Types
    typedef typename thrust::device_ptr< T > iterator; 
	typedef typename  T value_type;
    // Properties
    thrust::device_ptr< T > _ptr;
    size_t                  _size;
    size_t                  _capacity; 
    bool                    _owned; 
    
    DevVector( ) : _size( 0 ), _capacity( 0 ) {}
    
    DevVector( size_t n ) : _size( 0 ), _capacity( 0 )
    {
        resize( n ); 
        return;
    }

    DevVector( size_t n, T value ) : _size( 0 ), _capacity( 0 )
    {
        assign( n, value );
        return;
    }

    // Reuse the storage space
    DevVector( const DevVector<T> &clone ) : _size( 0 ), _owned( false )
    {
        _ptr        = clone._ptr; 
        _capacity   = clone._capacity; 
    }

    template< typename T1 >
    DevVector( const DevVector<T1> &clone ) : _size( 0 ), _owned( false )
    {
        _ptr        = thrust::device_ptr< T >( ( T* ) clone._ptr.get() ); 
        _capacity   = clone._capacity * sizeof( T1 ) / sizeof( T ); 
    }

    ~DevVector()
    {
        free();
        return;
    }

    void free() 
    {
        if ( _capacity > 0 && _owned )
            CudaSafeCall( cudaFree( _ptr.get() ) );

        _size       = 0; 
        _capacity   = 0; 

        return;
    }

    // Use only for cases where new size is within capacity
    // So, old data remains in-place
    void expand( size_t n )
    {
        assert( ( _capacity >= n ) && "New size not within current capacity! Use resize!" );
        _size = n;
    }

    // Resize with data remains
    void grow( size_t n ) 
    {
        assert( ( n >= _size ) && "New size not larger than old size." );

        if ( _capacity >= n )
        {
            _size = n; 
            return;
        }

        DevVector< T > tempVec( n ); 
        thrust::copy( begin(), end(), tempVec.begin() ); 
        swapAndFree( tempVec ); 
    }

    void resize( size_t n )
    {
        if ( _capacity >= n )
        {
            _size = n; 
            return;
        }

        free(); 

        _size       = n; 
        _capacity   = ( n == 0 ) ? 1 : n; 
        _owned      = true; 

        try
        {
            _ptr = thrust::device_malloc< T >( _capacity );
        }
        catch( ... )
        {
            // output an error message and exit
            const int OneMB = ( 1 << 20 );
            std::cerr << "thrust::device_malloc failed to allocate " << (sizeof( T ) * _capacity) / OneMB << " MB!" << std::endl;
			LOG_ERROR("thrust::device_malloc failed to allocate %.3f MB!", (sizeof(T) * _capacity) / OneMB);
            std::cerr << "size = " << _size << " sizeof(T) = " << sizeof( T ) << std::endl; 
            exit( -1 );
        }

        return;
    }

    void assign( size_t n, const T& value )
    {
        resize( n ); 
        thrust::fill_n( begin(), n, value );
        return;
    }

    size_t size() const { return _size; }
    size_t capacity() const { return _capacity; }

    thrust::device_reference< T > operator[] ( const size_t index ) const
    {
        return _ptr[ index ]; 
    }

    const iterator begin() const { return _ptr; }

    const iterator end() const { return _ptr + _size; }

    void erase( const iterator& first, const iterator& last )
    {
        if ( last == end() )
        {
            _size -= (last - first);
        }
        else
        {
            assert( false && "Not supported right now!" );
        }

        return;
    }

    void swap( DevVector< T >& arr ) 
    {
        size_t tempSize = _size; 
        size_t tempCap  = _capacity; 
        bool tempOwned  = _owned; 
        T* tempPtr      = ( _capacity > 0 ) ? _ptr.get() : 0; 

        _size       = arr._size; 
        _capacity   = arr._capacity; 
        _owned      = arr._owned; 

        if ( _capacity > 0 )
        {
            _ptr = thrust::device_ptr< T >( arr._ptr.get() ); 
        }

        arr._size       = tempSize; 
        arr._capacity   = tempCap; 
        arr._owned      = tempOwned; 

        if ( tempCap > 0 )
        {
            arr._ptr = thrust::device_ptr< T >( tempPtr );
        }

        return;
    }
    
    // Input array is freed
    void swapAndFree( DevVector< T >& inArr )
    {
        swap( inArr );
        inArr.free();
        return;
    }

    void copyFrom( const DevVector< T >& inArr )
    {
        resize( inArr.size() );
        thrust::copy( inArr.begin(), inArr.end(), begin() );
        return;
    }

    void fill( const T& value )
    {
        thrust::fill_n( _ptr, _size, value );
        return;
    }

    void copyToHost( thrust::host_vector< T >& dest )
    {
        dest.insert( dest.begin(), begin(), end() );
        return;
    }

    // Do NOT remove! Useful for debugging.
    void copyFromHost( const thrust::host_vector< T >& inArr )
    {
        resize( inArr.size() );
        thrust::copy( inArr.begin(), inArr.end(), begin() );
        return;
    }

    DevVector& operator=( DevVector& src )
    {
        resize( src._size ); 

        if ( src._size > 0 )
        {
            thrust::copy( src.begin(), src.end(), begin() ); 
        }

        return *this; 
    }
};

////////////////////////////////////////////////////////// Device containers //
typedef DevVector< char >     CharDVec;
typedef DevVector< int >      IntDVec;
typedef DevVector< int2 >     Int2DVec;

#endif // _DEV_VECTOR_H

