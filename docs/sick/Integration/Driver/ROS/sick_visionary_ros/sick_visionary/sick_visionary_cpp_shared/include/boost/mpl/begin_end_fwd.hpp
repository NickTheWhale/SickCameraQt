
#ifndef BOOST_MPL_BEGIN_END_FWD_HPP_INCLUDED
#define BOOST_MPL_BEGIN_END_FWD_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: begin_end_fwd.hpp 13472 2017-08-22 07:53:44Z richean $
// $Date: 2017-08-22 09:53:44 +0200 (Di, 22 Aug 2017) $
// $Revision: 13472 $

namespace boost { namespace mpl {

template< typename Tag > struct begin_impl;
template< typename Tag > struct end_impl;

template< typename Sequence > struct begin;
template< typename Sequence > struct end;

}}

#endif // BOOST_MPL_BEGIN_END_FWD_HPP_INCLUDED
