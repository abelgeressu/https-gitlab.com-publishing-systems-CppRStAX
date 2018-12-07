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
 * @file $/QName.h
 * @author Stephan Kreutzer
 * @since 2017-08-24
 */

#ifndef _CPPRSTAX_QNAME
#define _CPPRSTAX_QNAME

#include <string>

namespace cpprstax
{

class QName
{
public:
    QName(const std::string& namespaceURI, const std::string& localPart, const std::string& prefix);

public:
    const std::string& getNamespaceURI() const;
    const std::string& getLocalPart() const;
    const std::string& getPrefix() const;

public:
    bool operator==(const QName& rhs) const;

protected:
    /** @todo These should be smart pointers too, shouldn't they? */
    std::string m_strNamespaceURI;
    std::string m_strLocalPart;
    std::string m_strPrefix;

};

}

#endif
