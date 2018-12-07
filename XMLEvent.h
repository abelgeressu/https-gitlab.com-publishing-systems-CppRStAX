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
 * @file $/XMLEvent.h
 * @author Stephan Kreutzer
 * @since 2017-08-24
 */

#ifndef _CPPRSTAX_XMLEVENT_H
#define _CPPRSTAX_XMLEVENT_H

#include "StartElement.h"
#include "EndElement.h"
#include "Characters.h"
#include "Comment.h"
#include "ProcessingInstruction.h"
#include <memory>

namespace cpprstax
{

class XMLEvent
{
public:
    XMLEvent(std::unique_ptr<StartElement> pStartElement,
             std::unique_ptr<EndElement> pEndElement,
             std::unique_ptr<Characters> pCharacters,
             std::unique_ptr<Comment> pComment,
             std::unique_ptr<ProcessingInstruction> pProcessingInstruction);

public:
    bool isStartElement();
    StartElement& asStartElement();
    bool isEndElement();
    EndElement& asEndElement();
    bool isCharacters();
    Characters& asCharacters();
    bool isComment();
    Comment& asComment();
    bool isProcessingInstruction();
    ProcessingInstruction& asProcessingInstruction();

protected:
    std::unique_ptr<StartElement> m_pStartElement;
    std::unique_ptr<EndElement> m_pEndElement;
    std::unique_ptr<Characters> m_pCharacters;
    std::unique_ptr<Comment> m_pComment;
    std::unique_ptr<ProcessingInstruction> m_pProcessingInstruction;

};

}

#endif
