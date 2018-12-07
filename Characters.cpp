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
 * @file $/Characters.cpp
 * @author Stephan Kreutzer
 * @since 2017-09-01
 */

#include "Characters.h"

namespace cpprstax
{

Characters::Characters(std::unique_ptr<std::string> pData):
  m_pData(std::move(pData)),
  m_bIsWhiteSpace(true)
{
    if (m_pData == nullptr)
    {
        throw new std::invalid_argument("Nullptr passed.");
    }

    for (char& cCharacter : *m_pData)
    {
        if (std::isspace(cCharacter, m_aLocale) == 0)
        {
            m_bIsWhiteSpace = false;
            break;
        }
    }
}

const std::string& Characters::getData() const
{
    return *m_pData;
}

const bool& Characters::isWhiteSpace() const
{
    return m_bIsWhiteSpace;
}

}
