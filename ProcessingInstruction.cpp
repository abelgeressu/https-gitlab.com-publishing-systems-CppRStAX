/* Copyright (C) 2018 Stephan Kreutzer
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
 * @file $/ProcessingInstruction.cpp
 * @author Stephan Kreutzer
 * @since 2018-01-19
 */

#include "ProcessingInstruction.h"

namespace cpprstax
{

ProcessingInstruction::ProcessingInstruction(std::unique_ptr<std::string> pTarget,
                                             std::unique_ptr<std::string> pData):
  m_pTarget(std::move(pTarget)),
  m_pData(std::move(pData))
{
    if (m_pTarget == nullptr)
    {
        throw new std::invalid_argument("Nullptr passed.");
    }

    if (m_pData == nullptr)
    {
        throw new std::invalid_argument("Nullptr passed.");
    }
}

const std::string& ProcessingInstruction::getData() const
{
    return *m_pData;
}

const std::string& ProcessingInstruction::getTarget() const
{
    return *m_pTarget;
}

}
