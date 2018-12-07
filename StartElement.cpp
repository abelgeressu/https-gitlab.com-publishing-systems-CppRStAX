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
 * @file $/StartElement.cpp
 * @author Stephan Kreutzer
 * @since 2017-08-26
 */

#include "StartElement.h"

namespace cpprstax
{

StartElement::StartElement(std::unique_ptr<QName> pName, std::unique_ptr<std::list<std::unique_ptr<Attribute>>> pAttributes):
  m_pName(std::move(pName)),
  m_pAttributes(std::make_shared<std::list<std::shared_ptr<Attribute>>>())
{
    if (m_pName == nullptr)
    {
        throw new std::invalid_argument("Nullptr passed.");
    }

    if (pAttributes != nullptr)
    {
        for (std::list<std::unique_ptr<Attribute>>::iterator iter = pAttributes->begin();
             iter != pAttributes->end();
             iter++)
        {
            if (*iter != nullptr)
            {
                m_pAttributes->push_back(std::move(*iter));
            }
        }
    }
}

/**
 * @retval nullptr In case the attribute couldn't be found.
 */
const std::shared_ptr<Attribute> StartElement::getAttributeByName(const QName& aName) const
{
    for (std::list<std::shared_ptr<Attribute>>::iterator iter = m_pAttributes->begin();
          iter != m_pAttributes->end();
          iter++)
    {
        if ((*iter)->getName() == aName)
        {
            return *iter;
        }
    }

    return nullptr;
}

const std::shared_ptr<std::list<std::shared_ptr<Attribute>>> StartElement::getAttributes() const
{
    return m_pAttributes;
}

const QName& StartElement::getName() const
{
    return *m_pName;
}

}
