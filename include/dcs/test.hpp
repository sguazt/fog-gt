/**
 * \file dcs/test.hpp
 *
 * \brief Minimal framework for test suites.
 *
 * Basic usage:
 * <pre>
 * DCS_TEST_DEF( test_case_1 )
 * {
 *   // do your test stuff
 * }
 *
 * DCS_TEST_DEF( test_case_2 )
 * {
 *   // do your test stuff
 * }
 *
 * // ...
 *
 * DCS_TEST_DEF( test_case_n )
 * {
 *   // do your test stuff
 * }
 *
 * int main()
 * {
 *   DCS_TEST_SUITE( "My Test Suite" ); // optional
 *
 *   DCS_TEST_BEGIN()
 *     DCS_TEST_DO( test_case_1 );
 *     DCS_TEST_DO( test_case_2 );
 *     // ...
 *     DCS_TEST_DO( test_case_n );
 *   DCS_TEST_END()
 * }
 * </pre>
 * Inside each <em>test_case_<code>k</code></em> you can use the various
 * \c DCS_TEST* macros.
 *
 * \note To compile, make sure to add to the linker the standard math library.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 *
 * <hr/>
 *
 * Copyright 2009 Marco Guazzone (marco.guazzone@gmail.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DCS_TEST_HPP
#define DCS_TEST_HPP


#include <cmath>
#include <complex>
#include <cstddef>
#include <dcs/detail/promote_traits.hpp>
#include <iostream>
#include <limits>
#include <stdexcept>


namespace dcs { namespace test { namespace detail { namespace /*<unnamed>*/ {

/// Check if the given real number is 0.
template <typename T>
inline
bool iszero(T x)
{
	return ::std::fpclassify(x) == FP_ZERO;
}

/// Check if the given complex number is 0.
template <typename T>
inline
bool iszero(::std::complex<T> const& z)
{
	return iszero(z.real()) && iszero(z.imag());
}

/// Check if the given complex number is a NaN.
template <typename T>
inline
bool isnan(::std::complex<T> const& z)
{
	// According to IEEE, NaN is different even by itself
	return (z != z) || ::std::isnan(z.real()) || ::std::isnan(z.imag());
}

/// Check if two (real) numbers are close each other (wrt a given tolerance).
template <typename T1, typename T2, typename T3>
inline
bool close_to(T1 x, T2 y, T3 tol)
{
	typedef typename ::dcs::detail::promote_traits<typename ::dcs::detail::promote_traits<T1,T2>::promote_type,T3>::promote_type real_type;

	if (::std::isnan(x) || ::std::isnan(y))
	{
		// According to IEEE, NaN are different event by itself
		return false;
	}
	//return ::std::abs(x-y) <= (::std::max(static_cast<real_type>(::std::abs(x)), static_cast<real_type>(::std::abs(y)))*tol);
	real_type xx(::std::abs(x));
	real_type yy(::std::abs(y));
	return (::std::max(xx, yy)-::std::min(xx, yy)) <= tol;
}

/// Check if two complex numbers are close each other (wrt a given tolerance).
template <typename T1, typename T2, typename T3>
inline
bool close_to(::std::complex<T1> const& x, ::std::complex<T2> const& y, T3 tol)
{
	typedef typename ::dcs::detail::promote_traits<typename ::dcs::detail::promote_traits<T1,T2>::promote_type,T3>::promote_type real_type;

	if (isnan(x) || isnan(y))
	{
		// According to IEEE, NaN is different even by itself
		return false;
	}
	//::std::complex<real_type> xx(x);
	//::std::complex<real_type> yy(y);
	//return ::std::abs(xx-yy) <= (::std::max(::std::abs(xx), ::std::abs(yy))*tol);
	real_type xx(::std::abs(x));
	real_type yy(::std::abs(y));
	return (::std::max(xx, yy)-::std::min(xx, yy)) <= tol;
}

/// Check if two (real) numbers are close each other (wrt a given tolerance).
template <typename T1, typename T2, typename T3>
inline
bool rel_close_to(T1 x, T2 y, T3 tol)
{
	//typedef typename ::dcs::detail::promote_traits<typename ::dcs::detail::promote_traits<T1,T2>::promote_type,T3>::promote_type real_type;

	if (::std::isnan(x) || ::std::isnan(y))
	{ // According to IEEE, NaN is different even by itself
		return false;
	}
	if (iszero(x) && iszero(y))
	{
		return true;
	}
	return ::std::abs(x-y)/::std::abs(y) <= tol;
}

/// Check if two complex numbers are close each other (wrt a given tolerance).
template <typename T1, typename T2, typename T3>
inline
bool rel_close_to(::std::complex<T1> const& x, ::std::complex<T2> const& y, T3 tol)
{
	typedef typename ::dcs::detail::promote_traits<typename ::dcs::detail::promote_traits<T1,T2>::promote_type,T3>::promote_type real_type;

	if (isnan(x) || isnan(y))
	{
		// According to IEEE, NaN is different even by itself
		return false;
	}
	::std::complex<real_type> xx(x);
	::std::complex<real_type> yy(y);
	return ::std::abs(xx-yy)/::std::abs(yy) <= tol;
}

}}}} // Namespace dcs::test::detail::<unnamed>


/// Expand its argument \a x.
#define DCS_TEST_PARAM_EXPAND_(x) x


/// Expand its argument \a x inside parenthesis.
#define DCS_TEST_PARAM_EXPANDP_(x) (x)


/// Transform its argument \a x into a string.
#define DCS_TEST_STRINGIFY_(x) #x


/// Concatenate its two \e string arguments \a x and \a y.
#define DCS_TEST_STR_JOIN_(x,y) x ## y


/// Define the name of the entire test suite.
#define DCS_TEST_SUITE(m) ::std::clog << "--- Test Suite: " << DCS_TEST_PARAM_EXPAND_(m) << " ---" << ::std::endl;


/// Define the name of a single test case.
#define DCS_TEST_CASE(m) ::std::clog << "--- Test Case: " << DCS_TEST_PARAM_EXPAND_(m) << " ---" << ::std::endl;


/// Init procedure.
#define DCS_TEST_INIT() static void dcs_test_init__()


/// Define the beginning of the test suite.
#define DCS_TEST_BEGIN()	/* [DCS_TEST_BEGIN] */ \
							{ \
								/* Begin of Test Suite */ \
								::std::size_t test_fails__(0); \
							/* [/DCS_TEST_BEGIN] */


/// Define a new test case \a x inside the current test suite.
#define DCS_TEST_DEF(x) static void DCS_TEST_PARAM_EXPAND_(x)(::std::size_t& test_fails__)


/// Call the previously defined init function.
#define DCS_TEST_DO_INIT()  dcs_test_init__()


/// Call a previously defined test case \a x.
#define DCS_TEST_DO(x)	/* [DCS_TEST_DO] */ \
						try \
						{ \
							DCS_TEST_PARAM_EXPAND_(x)(test_fails__); \
						} \
						catch (::std::exception& e) \
						{ \
							++test_fails__; \
							DCS_TEST_ERROR( e.what() ); \
						} \
						catch (...) \
						{ \
							++test_fails__; \
						} \
						/* [/DCS_TEST_DO] */


/// Define the end of the test suite.
#define DCS_TEST_END()	/* [DCS_TEST_END] */ \
							if (test_fails__ > 0) \
							{ \
								::std::clog << "Number of failed tests: " << test_fails__ << ::std::endl; \
							} \
							else \
							{ \
								::std::clog << "No failed test" << ::std::endl; \
							} \
							/* End of Test Suite */ \
						} \
						/* [/DCS_TEST_END] */


/// Output the message \a m.
/// Note: we don't use macro expansion inside parenthesis to let \a m be an
///  expression of the form <code>a &lt;&lt; b</code>.
#define DCS_TEST_TRACE(m) ::std::clog << "[Test>> " << DCS_TEST_PARAM_EXPAND_(m) << ::std::endl


/// Check for the truth of the given assertion \a x.
#define DCS_TEST_CHECK(x)	/* [DCS_TEST_CHECK] */ \
							if (!DCS_TEST_PARAM_EXPANDP_(x)) \
							{ \
								DCS_TEST_ERROR( "Failed assertion: " << DCS_TEST_STRINGIFY_(x) ); \
								++test_fails__; \
							} \
							/* [/DCS_TEST_CHECK] */


/// Check for the equality of \a x against \a y.
#define DCS_TEST_CHECK_EQ(x,y)	/* [DCS_TEST_CHECK_EQ] */ \
								if (!(DCS_TEST_PARAM_EXPANDP_(x) == DCS_TEST_PARAM_EXPANDP_(y))) \
								{ \
									DCS_TEST_ERROR( "Failed assertion: (" << DCS_TEST_STRINGIFY_(x) << " == " << DCS_TEST_STRINGIFY_(y) << ")" ); \
									++test_fails__; \
								} \
								/* [/DCS_TEST_CHECK_EQ] */


/// Alias for macro \c DCS_TEST_CHECK_EQ (for backward compatibility).
#define DCS_TEST_CHECK_EQUAL(x,y) DCS_TEST_CHECK_EQ(x,y)


/// Check that \a x and \a y are close with respect a given tolerance \a e.
#define DCS_TEST_CHECK_CLOSE(x,y,e)	/* [DCS_TEST_CHECK_CLOSE] */ \
									if (!::dcs::test::detail::close_to(DCS_TEST_PARAM_EXPAND_(x), DCS_TEST_PARAM_EXPAND_(y), DCS_TEST_PARAM_EXPAND_(e))) \
									{ \
										DCS_TEST_ERROR( "Failed assertion: abs(" << DCS_TEST_STRINGIFY_(x) << "-" << DCS_TEST_STRINGIFY_(y) << ") <= " << DCS_TEST_STRINGIFY_(e) << " [with " << DCS_TEST_STRINGIFY_(x) << " == " << DCS_TEST_PARAM_EXPANDP_(x) << ", " << DCS_TEST_STRINGIFY_(y) << " == " << DCS_TEST_PARAM_EXPANDP_(y) << " and " << DCS_TEST_STRINGIFY_(e) << " == " << DCS_TEST_PARAM_EXPANDP_(e) << "]" ); \
										++test_fails__; \
									} \
									/* [/DCS_TEST_CHECK_CLOSE] */


/// Check for the floating point relative precision of \a x against \a y with a
/// tolerance of \a e.
#define DCS_TEST_CHECK_REL_CLOSE(x,y,e)	/* [DCS_TEST_CHECK_REL_CLOSE] */ \
										if (!::dcs::test::detail::rel_close_to(DCS_TEST_PARAM_EXPAND_(x), DCS_TEST_PARAM_EXPAND_(y), DCS_TEST_PARAM_EXPAND_(e))) \
										{ \
											DCS_TEST_ERROR( "Failed assertion: abs((" << DCS_TEST_STRINGIFY_(x) << "-" << DCS_TEST_STRINGIFY_(y) << ")/" << DCS_TEST_STRINGIFY_(y) << ") <= " << DCS_TEST_STRINGIFY_(e)  << " [with " << DCS_TEST_STRINGIFY_(x) << " == " << DCS_TEST_PARAM_EXPANDP_(x) << ", " << DCS_TEST_STRINGIFY_(y) << " == " << DCS_TEST_PARAM_EXPANDP_(y) << " and " << DCS_TEST_STRINGIFY_(e) << " == " << DCS_TEST_PARAM_EXPANDP_(e) << "]" ); \
											++test_fails__; \
										} \
										/* [/DCS_TEST_CHECK_REL_CLOSE] */


/// Check that elements of \a x and \a y are equal.
#define DCS_TEST_CHECK_VECTOR_EQ(x,y,n)	/* [DCS_TEST_CHECK_VECTOR_EQ] */ \
										if (DCS_TEST_PARAM_EXPANDP_(n) > 0) \
										{ \
											::std::size_t n__ = DCS_TEST_PARAM_EXPAND_(n); \
											for (::std::size_t i__ = n__; i__ > 0; --i__) \
											{ \
												if (!(DCS_TEST_PARAM_EXPANDP_(x)[n__-i__]==DCS_TEST_PARAM_EXPANDP_(y)[n__-i__])) \
												{ \
													DCS_TEST_ERROR( "Failed assertion: (" << DCS_TEST_STRINGIFY_(x[i__]) << "==" << DCS_TEST_STRINGIFY_(y[i__]) << ")" << " [with " << DCS_TEST_STRINGIFY_(x[i__]) << " == " << DCS_TEST_PARAM_EXPANDP_(x)[n__-i__] << ", " << DCS_TEST_STRINGIFY_(y[i__]) << " == " << DCS_TEST_PARAM_EXPANDP_(y)[n__-i__] << ", " << DCS_TEST_STRINGIFY_(i__) << " == " << i__ << " and " << DCS_TEST_STRINGIFY_(n) << " == " << n__ << "]" ); \
													++test_fails__; \
												} \
											} \
										} \
										/* [/DCS_TEST_CHECK_VECTOR_EQ] */


/// Check that elements of \a x and \a y are close with respect to a given
/// precision.
#define DCS_TEST_CHECK_VECTOR_CLOSE(x,y,n,e)	/* [DCS_TEST_CHECK_VECTOR_CLOSE] */ \
												if (DCS_TEST_PARAM_EXPANDP_(n) > 0) \
												{ \
													::std::size_t n__ = DCS_TEST_PARAM_EXPAND_(n); \
													for (::std::size_t i__ = n__; i__ > 0; --i__) \
													{ \
														if (!::dcs::test::detail::close_to(DCS_TEST_PARAM_EXPANDP_(x)[n__-i__], DCS_TEST_PARAM_EXPANDP_(y)[n__-i__], DCS_TEST_PARAM_EXPANDP_(e))) \
														{ \
															DCS_TEST_ERROR( "Failed assertion: abs(" << DCS_TEST_STRINGIFY_(x[i__]) << "-" << DCS_TEST_STRINGIFY_(y[i__]) << ") <= " << DCS_TEST_STRINGIFY_(e)  << " [with " << DCS_TEST_STRINGIFY_(x[i__]) << " == " << DCS_TEST_PARAM_EXPANDP_(x)[n__-i__] << ", " << DCS_TEST_STRINGIFY_(y[i__]) << " == " << DCS_TEST_PARAM_EXPANDP_(y)[n__-i__] << ", " << DCS_TEST_STRINGIFY_(i__) << " == " << i__ << ", " << DCS_TEST_STRINGIFY_(e) << " == " << DCS_TEST_PARAM_EXPANDP_(e) << " and " << DCS_TEST_STRINGIFY_(n) << " == " << n__ << "]" ); \
															++test_fails__; \
														} \
													} \
												} \
												/* [/DCS_TEST_CHECK_VECTOR_CLOSE] */


/// Check that elements of \a x and \a y are close with respect to a given
/// relative precision.
#define DCS_TEST_CHECK_VECTOR_REL_CLOSE(x,y,n,e)	/* [DCS_TEST_CHECK_VECTOR_REL_CLOSE] */ \
													if (DCS_TEST_PARAM_EXPANDP_(n) > 0) \
													{ \
														::std::size_t n__ = DCS_TEST_PARAM_EXPAND_(n); \
														for (::std::size_t i__ = n__; i__ > 0; --i__) \
														{ \
															if (!::dcs::test::detail::rel_close_to(DCS_TEST_PARAM_EXPANDP_(x)[n__-i__], DCS_TEST_PARAM_EXPANDP_(y)[n__-i__], DCS_TEST_PARAM_EXPANDP_(e))) \
															{ \
																DCS_TEST_ERROR( "Failed assertion: abs((" << DCS_TEST_STRINGIFY_(x[i__]) << "-" << DCS_TEST_STRINGIFY_(y[i__]) << ")/" << DCS_TEST_STRINGIFY_(y[i__]) << ") <= " << DCS_TEST_STRINGIFY_(e)  << " [with " << DCS_TEST_STRINGIFY_(x[i__]) << " == " << DCS_TEST_PARAM_EXPANDP_(x)[n__-i__] << ", " << DCS_TEST_STRINGIFY_(y[i__]) << " == " << DCS_TEST_PARAM_EXPANDP_(y)[n__-i__] << ", " << DCS_TEST_STRINGIFY_(i__) << " == " << i__ << ", " << DCS_TEST_STRINGIFY_(e) << " == " << DCS_TEST_PARAM_EXPANDP_(e) << " and " << DCS_TEST_STRINGIFY_(n) << " == " << n__ << "]" ); \
																++test_fails__; \
															} \
														} \
													} \
													/* [/DCS_TEST_CHECK_VECTOR_REL_CLOSE] */


/// Check that elements of matrices \a x and \a y are equal.
#define DCS_TEST_CHECK_MATRIX_EQ(x,y,nr,nc)	/* [DCS_TEST_CHECK_MATRIX_EQ] */ \
											for (::std::size_t i__ = 0; i__ < DCS_TEST_PARAM_EXPAND_(nr); ++i__) \
											{ \
												for (::std::size_t j__ = 0; j__ < DCS_TEST_PARAM_EXPAND_(nc); ++j__) \
												{ \
													if (!(DCS_TEST_PARAM_EXPANDP_(x)(i__,j__)==DCS_TEST_PARAM_EXPANDP_(y)(i__,j__))) \
													{ \
														DCS_TEST_ERROR( "Failed assertion: (" << DCS_TEST_STRINGIFY_(x(i__,j__)) << " == " << DCS_TEST_STRINGIFY_(y(i__,j__)) << ") [with " << DCS_TEST_STRINGIFY_(x(i__,j__)) << " == " << DCS_TEST_PARAM_EXPANDP_(x)(i__,j__) << ", " << DCS_TEST_STRINGIFY_(y(i__,j__)) << " == " << DCS_TEST_PARAM_EXPANDP_(y)(i__,j__) << ", " << DCS_TEST_STRINGIFY_(i__) << " == " << i__ << ", " << DCS_TEST_STRINGIFY_(j__) << " == " << DCS_TEST_PARAM_EXPANDP_(j__) << ", " << DCS_TEST_STRINGIFY_(nr) << " == " << DCS_TEST_PARAM_EXPANDP_(nr) << " and " << DCS_TEST_STRINGIFY_(nc) << " == " << DCS_TEST_PARAM_EXPANDP_(nc) << "]" ); \
														++test_fails__; \
													} \
												} \
											} \
											/* [/DCS_TEST_CHECK_MATRIX_EQ] */


/// Check that elements of matrices \a x and \a y are are with respect to a
/// given precision.
#define DCS_TEST_CHECK_MATRIX_CLOSE(x,y,nr,nc, e)	/* [DCS_TEST_CHECK_MATRIX_CLOSE] */ \
													for (::std::size_t i__ = 0; i__ < DCS_TEST_PARAM_EXPANDP_(nr); ++i__) \
													{ \
														for (::std::size_t j__ = 0; j__ < DCS_TEST_PARAM_EXPANDP_(nc); ++j__) \
														{ \
															if (!::dcs::test::detail::close_to(DCS_TEST_PARAM_EXPANDP_(x)(i__,j__), DCS_TEST_PARAM_EXPANDP_(y)(i__,j__), DCS_TEST_PARAM_EXPANDP_(e))) \
															{ \
																DCS_TEST_ERROR( "Failed assertion: abs(" << DCS_TEST_STRINGIFY_(x(i__,j__)) << "-" << DCS_TEST_STRINGIFY_(y(i__,j__)) << ") <= " << DCS_TEST_STRINGIFY_(e)  << " [with " << DCS_TEST_STRINGIFY_(x(i__,j__)) << " == " << DCS_TEST_PARAM_EXPANDP_(x)(i__,j__) << ", " << DCS_TEST_STRINGIFY_(y(i__,j__)) << " == " << DCS_TEST_PARAM_EXPANDP_(y)(i__,j__) << ", " << DCS_TEST_STRINGIFY_(i__) << " == " << i__ << ", " << DCS_TEST_STRINGIFY_(j__) << " == " << DCS_TEST_PARAM_EXPANDP_(j__) << ", " << DCS_TEST_STRINGIFY_(e) << " == " << DCS_TEST_PARAM_EXPANDP_(e) << ", " << DCS_TEST_STRINGIFY_(nr) << " == " << DCS_TEST_PARAM_EXPANDP_(nr) << " and " << DCS_TEST_STRINGIFY_(nc) << " == " << DCS_TEST_PARAM_EXPANDP_(nc) << "]" ); \
																++test_fails__; \
															} \
														} \
													} \
													/* [/DCS_TEST_CHECK_MATRIX_CLOSE] */


/// Check that elements of matrices \a x and \a y are are with respect to a
/// given relative precision.
#define DCS_TEST_CHECK_MATRIX_REL_CLOSE(x,y,nr,nc, e)	/* [DCS_TEST_CHECK_MATRIX_REL_CLOSE] */ \
														for (::std::size_t i__ = 0; i__ < DCS_TEST_PARAM_EXPANDP_(nr); ++i__) \
														{ \
															for (::std::size_t j__ = 0; j__ < DCS_TEST_PARAM_EXPANDP_(nc); ++j__) \
															{ \
																if (!::dcs::test::detail::rel_close_to(DCS_TEST_PARAM_EXPANDP_(x)(i__,j__), DCS_TEST_PARAM_EXPANDP_(y)(i__,j__), DCS_TEST_PARAM_EXPANDP_(e))) \
																{ \
																	DCS_TEST_ERROR( "Failed assertion: abs((" << DCS_TEST_STRINGIFY_(x(i__,j__)) << "-" << DCS_TEST_STRINGIFY_(y(i__,j__)) << ")/" << DCS_TEST_STRINGIFY_(y(i__,j__)) << ") <= " << DCS_TEST_STRINGIFY_(e)  << " [with " << DCS_TEST_STRINGIFY_(x(i__,j__)) << " == " << DCS_TEST_PARAM_EXPANDP_(x)(i__,j__) << ", " << DCS_TEST_STRINGIFY_(y(i__,j__)) << " == " << DCS_TEST_PARAM_EXPANDP_(y)(i__,j__) << ", " << DCS_TEST_STRINGIFY_(i__) << " == " << i__ << ", " << DCS_TEST_STRINGIFY_(j__) << " == " << DCS_TEST_PARAM_EXPANDP_(j__) << ", " << DCS_TEST_STRINGIFY_(e) << " == " << DCS_TEST_PARAM_EXPANDP_(e) << ", " << DCS_TEST_STRINGIFY_(nr) << " == " << DCS_TEST_PARAM_EXPANDP_(nr) << " and " << DCS_TEST_STRINGIFY_(nc) << " == " << DCS_TEST_PARAM_EXPANDP_(nc) << "]" ); \
																	++test_fails__; \
																} \
															} \
														} \
														/* [/DCS_TEST_CHECK_MATRIX_REL_CLOSE] */


/// Output the error message \a x.
#define DCS_TEST_ERROR(x) ::std::clog << "[Error (" << __FILE__ << ":" << __func__ << ":" << __LINE__ << ")>> " << DCS_TEST_PARAM_EXPAND_(x) << ::std::endl

#endif // DCS_TEST_HPP
