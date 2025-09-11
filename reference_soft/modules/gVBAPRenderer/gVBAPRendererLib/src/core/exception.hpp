/***********************************************************************************
 
 This software module was originally developed by 
 
 IOSONO GmbH
 
 in the course of development of the ISO/IEC 23008-3 for reference purposes and its 
 performance may not have been optimized. This software module is an implementation
 of one or more tools as specified by the ISO/IEC 23008-3 standard. ISO/IEC gives 
 you a royalty-free, worldwide, non-exclusive, copyright license to copy, distribute, 
 and make derivative works of this software module or modifications  thereof for use
 in implementations or products claiming conformance to the ISO/IEC 23008-3 standard 
 and which satisfy any specified conformance criteria. Those intending to use this 
 software module in products are advised that its use may infringe existing patents. 
 ISO/IEC have no liability for use of this software module or modifications thereof. 
 Copyright is not released for products that do not conform to the ISO/IEC 23008-3 
 standard.
 
 IOSONO GmbH retains full right to modify and use the code for its own purpose,
 assign or donate the code to a third party and to inhibit third parties from using 
 the code for products that do not conform to MPEG-related ITU Recommendations and/or 
 ISO/IEC International Standards.
 
 This copyright notice must be included in all copies or derivative works. 
 
 Copyright (c) ISO/IEC 2013.
 
 ***********************************************************************************/

#ifndef __DEFAULT_RENDERER_EXEPTION_HPP__
#define __DEFAULT_RENDERER_EXEPTION_HPP__


//-------------------------------------------------------------------------------------------------

// Stl headers:
#include <exception>
#include <string>
#include <sstream>
#include <iostream>

//-------------------------------------------------------------------------------------------------

/**
 * @brief Use this marco to throw an exception.
 *
 * @param[in] className The exception class which should be throw.
 * @param[in] message   The string for the exception.
 */
#define THROW_MPEGH_EXCEPTION(className, message)\
{\
  std::ostringstream I0S0N0_MPEGH_ThrowMessageStream;\
  I0S0N0_MPEGH_ThrowMessageStream << message;\
  className I0S0N0_MPEGH_throwException(I0S0N0_MPEGH_ThrowMessageStream.str(), \
                                 __FILE__,                       \
                                 __LINE__);        \
  throw I0S0N0_MPEGH_throwException;\
}

//-------------------------------------------------------------------------------------------------

namespace iosono
{
namespace mpeg
{

//-------------------------------------------------------------------------------------------------

/**
 * @brief Base class for all exceptions in iosono space.
 */
class Exception : public std::exception
{
public:

  /**
   * @brief Constructor.
   *
   * @param[in] message The message text.
   * @param[in] type    The type of the message as string.
   */
  explicit Exception(const std::string& message,
                     const std::string& file = "",
                     unsigned int line = 0,
                     const std::string& type = "Exception")
    : m_fileName(file), m_line(line), m_type(type)
  {
#ifdef NDEBUG
    m_message = message;
#else
    std::ostringstream sstrm;
    sstrm << message << "[" << m_fileName << ":"<< m_line << "]\n";
    m_message = sstrm.str();
    std::cout << "!!!!!!! Exception: " << m_message << "!!!!!!!" << std::endl;
#endif
  }

  /**
   * @brief Destructor.
   */
 virtual ~Exception() throw() { }

  /**
   * @brief Returns the message text of the exception.
   */
  const std::string& getMessage() const { return m_message; }

  /**
   * @brief Returns the type name of this exception.
   */
  const std::string& getType() const { return m_type; }

  /**
   * @brief Returns the message text of the exception.
   */
  virtual const char* what() const throw() { return m_message.c_str(); }

 /**
  * @brief Returns the name of the file where the exception was thrown
  */
  const std::string& getFile() const { return m_fileName; }

 /**
  * @brief Returns the code line where the exception was thrown
  */
  unsigned int getLine() const { return m_line; }

private:

  /// The message text for this exception.
  std::string m_message;

  /// The file where the exception was thrown
  std::string m_fileName;

  /// The code line where the exception was thrown
  unsigned int m_line;

  /// The name of the type of this exception.
  std::string m_type;

};

//-------------------------------------------------------------------------------------------------

/**
 * @brief Pipe operator for the exception.
 */
inline
std::ostream&
operator <<(std::ostream& os, const Exception& exception)
{
  os << std::flush;
  if(exception.what())
  {
    os << exception.what();
  }
  os << std::flush;
  return os;
}

//-------------------------------------------------------------------------------------------------

} // namespace mpeg
} // namespace iosono


#endif
