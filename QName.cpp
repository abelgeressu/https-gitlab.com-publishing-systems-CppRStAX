/* Copyright (C) 2017-2018 Stephan Kreutzer
 *
 * This file is part of CppRStAX.
 *
 * CppRStAX is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License version 3 or any later
 * version of the license, as published by the Free Software Foundation.
 *
 * CppRStAX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License 3 for more details.
 *
 * You should have received a copy of the GNU Affero General Public License 3
 * along with CppRStAX. If not, see <http://www.gnu.org/licenses/>.
 */
/**
 * @file $/QName.cpp
 * @author Stephan Kreutzer
 * @since 2017-08-24
 */

#include "QName.h"

namespace cpprstax
{

QName::QName(const std::string& namespaceURI, const std::string& localPart, const std::string& prefix):
  m_strNamespaceURI(namespaceURI), m_strLocalPart(localPart), m_strPrefix(prefix)
{

}

const std::string& QName::getNamespaceURI() const
{
    return m_strNamespaceURI;
}

const std::string& QName::getLocalPart() const
{
    return m_strLocalPart;
}
const std::string& QName::getPrefix() const
{
    return m_strPrefix;
}

bool QName::operator==(const QName& rhs) const
{
    return m_strLocalPart == rhs.getLocalPart() &&
           m_strPrefix == rhs.getPrefix() &&
           m_strNamespaceURI == rhs.getNamespaceURI();
}

}
