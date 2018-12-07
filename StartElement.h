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
 * @file $/StartElement.h
 * @author Stephan Kreutzer
 * @since 2017-08-26
 */

#ifndef _CPPRSTAX_STARTELEMENT_H
#define _CPPRSTAX_STARTELEMENT_H

#include "QName.h"
#include "Attribute.h"
#include <memory>
#include <list>

namespace cpprstax
{

class StartElement
{
public:
    StartElement(std::unique_ptr<QName> pName, std::unique_ptr<std::list<std::unique_ptr<Attribute>>> pAttributes);

public:
    const std::shared_ptr<Attribute> getAttributeByName(const QName& aName) const;
    const std::shared_ptr<std::list<std::shared_ptr<Attribute>>> getAttributes() const;
    const QName& getName() const;

protected:
    std::unique_ptr<QName> m_pName;
    std::shared_ptr<std::list<std::shared_ptr<Attribute>>> m_pAttributes;

};

}

#endif
