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
 * @file $/EndElement.cpp
 * @author Stephan Kreutzer
 * @since 2017-08-28
 */

#include "EndElement.h"

namespace cpprstax
{

EndElement::EndElement(std::unique_ptr<QName> pName):
  m_pName(std::move(pName))
{
    if (m_pName == nullptr)
    {
        throw new std::invalid_argument("Nullptr passed.");
    }
}

const QName& EndElement::getName() const
{
    return *m_pName;
}

}
