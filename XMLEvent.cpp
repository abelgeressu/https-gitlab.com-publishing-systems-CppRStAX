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
 * @file $/XMLEvent.cpp
 * @author Stephan Kreutzer
 * @since 2017-08-24
 */

#include "XMLEvent.h"

namespace cpprstax
{

XMLEvent::XMLEvent(std::unique_ptr<StartElement> pStartElement,
                   std::unique_ptr<EndElement> pEndElement,
                   std::unique_ptr<Characters> pCharacters,
                   std::unique_ptr<Comment> pComment,
                   std::unique_ptr<ProcessingInstruction> pProcessingInstruction):
  m_pStartElement(std::move(pStartElement)),
  m_pEndElement(std::move(pEndElement)),
  m_pCharacters(std::move(pCharacters)),
  m_pComment(std::move(pComment)),
  m_pProcessingInstruction(std::move(pProcessingInstruction))
{
    int nPointerCount = 0;

    if (m_pStartElement != nullptr)
    {
        ++nPointerCount;
    }

    if (m_pEndElement != nullptr)
    {
        ++nPointerCount;
    }

    if (m_pCharacters != nullptr)
    {
        ++nPointerCount;
    }

    if (m_pComment != nullptr)
    {
        ++nPointerCount;
    }

    if (m_pProcessingInstruction != nullptr)
    {
        ++nPointerCount;
    }

    if (nPointerCount != 1)
    {
        throw new std::invalid_argument("XMLEvent constructor expects exactly 1 parameter to be set.");
    }
}

bool XMLEvent::isStartElement()
{
    return m_pStartElement != nullptr;
}

StartElement& XMLEvent::asStartElement()
{
    if (m_pStartElement == nullptr)
    {
        throw new std::logic_error("Isn't a StartElement.");
    }

    return *m_pStartElement;
}

bool XMLEvent::isEndElement()
{
    return m_pEndElement != nullptr;
}

EndElement& XMLEvent::asEndElement()
{
    if (m_pEndElement == nullptr)
    {
        throw new std::logic_error("Isn't an EndElement.");
    }

    return *m_pEndElement;
}

bool XMLEvent::isCharacters()
{
    return m_pCharacters != nullptr;
}

Characters& XMLEvent::asCharacters()
{
    if (m_pCharacters == nullptr)
    {
        throw new std::logic_error("Isn't Characters.");
    }

    return *m_pCharacters;
}

bool XMLEvent::isComment()
{
    return m_pComment != nullptr;
}

Comment& XMLEvent::asComment()
{
    if (m_pComment == nullptr)
    {
        throw new std::logic_error("Isn't Comment.");
    }

    return *m_pComment;
}

bool XMLEvent::isProcessingInstruction()
{
    return m_pProcessingInstruction != nullptr;
}

ProcessingInstruction& XMLEvent::asProcessingInstruction()
{
    if (m_pProcessingInstruction == nullptr)
    {
        throw new std::logic_error("Isn't ProcessingInstruction.");
    }

    return *m_pProcessingInstruction;
}

}
