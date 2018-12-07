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
 * @file $/ProcessingInstruction.h
 * @author Stephan Kreutzer
 * @since 2018-01-19
 */

#ifndef _CPPRSTAX_PROCESSING_INSTRUCTION_H
#define _CPPRSTAX_PROCESSING_INSTRUCTION_H

#include <memory>
#include <string>

namespace cpprstax
{

class ProcessingInstruction
{
public:
    ProcessingInstruction(std::unique_ptr<std::string> pTarget,
                          std::unique_ptr<std::string> pData);

public:
    const std::string& getData() const;
    const std::string& getTarget() const;

protected:
    std::unique_ptr<std::string> m_pTarget;
    std::unique_ptr<std::string> m_pData;


};

}

#endif
