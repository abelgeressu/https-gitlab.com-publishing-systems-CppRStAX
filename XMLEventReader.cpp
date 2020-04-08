/* Copyright (C) 2017-2020 Stephan Kreutzer
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
 * @file $/XMLEventReader.cpp
 * @todo Check well-formedness: check if start and end tags match by having a
 *     stack of the current XPath tree location/level/hierarchy/position, and
 *     if only one root element, and if starting with an element, etc.
 * @todo There's no support for markup declarations except for comments. When
 *     reading in forward direction, "<!" can easily be identified as being
 *     a markup declaration, but their ends being a mere ">" doesn't give an
 *     indication for backwards/reverse reading, until everything has been
 *     read to the begin, and that conflicts with valid and invalid ordinary
 *     start and end tags, so the implicit call tree would have to make sure
 *     that all reading of assumed start/end tags that turn out to be a markup
 *     declaration at the end are properly handled.
 * @author Stephan Kreutzer
 * @since 2018-11-11
  */

#include "XMLEventReader.h"
#include "StartElement.h"
#include "EndElement.h"
#include "Characters.h"
#include "Comment.h"
#include "QName.h"
#include "Attribute.h"
#include <string>
#include <memory>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace cpprstax
{

XMLEventReader::XMLEventReader(std::istream& aStream):
  m_aStream(aStream),
  m_bHasNextCalled(false),
  m_bHasPreviousCalled(false),
  m_bEventsAreForwardDirection(true)
{
    m_aEntityReplacementDictionary.insert(std::pair<std::string, std::string>("amp", "&"));
    m_aEntityReplacementDictionary.insert(std::pair<std::string, std::string>("lt", "<"));
    m_aEntityReplacementDictionary.insert(std::pair<std::string, std::string>("gt", ">"));
    m_aEntityReplacementDictionary.insert(std::pair<std::string, std::string>("apos", "'"));
    m_aEntityReplacementDictionary.insert(std::pair<std::string, std::string>("quot", "\""));

    m_aREntityReplacementDictionary.insert(std::pair<std::string, std::string>("pma", "&"));
    m_aREntityReplacementDictionary.insert(std::pair<std::string, std::string>("tl", "<"));
    m_aREntityReplacementDictionary.insert(std::pair<std::string, std::string>("tg", ">"));
    m_aREntityReplacementDictionary.insert(std::pair<std::string, std::string>("sopa", "'"));
    m_aREntityReplacementDictionary.insert(std::pair<std::string, std::string>("touq", "\""));

    /** @todo Load more from a catalogue, which itself is written in XML and needs to be read
      * in here by another local XMLEventReader object, containing mappings from entity to
      * replacement characters. No need to deal with DTDs as they're non-XML, and extracting
      * those mappings from a DTD can be a separate program or manual procedure. Also see
      * intermediate workaround method XMLEventReader::addToEntityReplacementDictionary(). */
}

XMLEventReader::~XMLEventReader()
{

}

bool XMLEventReader::hasNext()
{
    if (m_bHasPreviousCalled == true ||
        m_bEventsAreForwardDirection != true)
    {
        std::queue<std::unique_ptr<XMLEvent>>().swap(m_aEvents);
        m_bHasPreviousCalled = false;
        m_bEventsAreForwardDirection = true;
    }

    if (m_aEvents.size() > 0)
    {
        return true;
    }

    if (m_bHasNextCalled == true)
    {
        return false;
    }
    else
    {
        m_bHasNextCalled = true;
    }

    char cByte('\0');
    m_aStream.get(cByte);

    if (m_aStream.eof() == true)
    {
        return false;
    }

    if (m_aStream.bad() == true)
    {
        throw new std::runtime_error("Stream is bad.");
    }

    if (cByte == '<')
    {
        return HandleTag();
    }
    else
    {
        return HandleText(cByte);
    }
}

std::unique_ptr<XMLEvent> XMLEventReader::nextEvent()
{
    if (m_bHasPreviousCalled == true ||
        m_bEventsAreForwardDirection != true)
    {
        throw new std::logic_error("Attempted XMLEventReader::nextEvent() without checking XMLEventReader::hasNext() first.");
    }

    if (m_aEvents.size() <= 0 &&
        m_bHasNextCalled == false)
    {
        if (hasNext() != true)
        {
            throw new std::logic_error("Attempted XMLEventReader::nextEvent() while there isn't one instead of checking XMLEventReader::hasNext() first.");
        }
    }

    m_bHasNextCalled = false;

    if (m_aEvents.size() <= 0)
    {
        throw new std::logic_error("XMLEventReader::nextEvent() while there isn't one, ignoring XMLEventReader::hasNext() == false.");
    }

    std::unique_ptr<XMLEvent> pEvent(std::move(m_aEvents.front()));
    m_aEvents.pop();

    return std::move(pEvent);
}

bool XMLEventReader::hasPrevious()
{
    if (m_bHasNextCalled == true ||
        m_bEventsAreForwardDirection == true)
    {
        std::queue<std::unique_ptr<XMLEvent>>().swap(m_aEvents);
        m_bHasNextCalled = false;
        m_bEventsAreForwardDirection = false;
    }

    if (m_aEvents.size() > 0)
    {
        return true;
    }

    if (m_bHasPreviousCalled == true)
    {
        return false;
    }
    else
    {
        m_bHasPreviousCalled = true;
    }

    char cByte('\0');

    if (rget(cByte) != true)
    {
        return false;
    }

    if (cByte == '>')
    {
        return HandleRTag();
    }
    else
    {
        return HandleRText(cByte);
    }
}

std::unique_ptr<XMLEvent> XMLEventReader::previousEvent()
{
    if (m_bHasNextCalled == true ||
        m_bEventsAreForwardDirection == true)
    {
        throw new std::logic_error("Attempted XMLEventReader::previousEvent() without checking XMLEventReader::hasPrevious() first.");
    }

    if (m_aEvents.size() <= 0 &&
        m_bHasPreviousCalled == false)
    {
        if (hasPrevious() != true)
        {
            throw new std::logic_error("Attempted XMLEventReader::previousEvent() while there isn't one instead of checking XMLEventReader::hasPrevious() first.");
        }
    }

    m_bHasPreviousCalled = false;

    if (m_aEvents.size() <= 0)
    {
        throw new std::logic_error("XMLEventReader::previousEvent() while there isn't one, ignoring XMLEventReader::hasPrevious() == false.");
    }

    std::unique_ptr<XMLEvent> pEvent(std::move(m_aEvents.front()));
    m_aEvents.pop();

    return std::move(pEvent);
}

int XMLEventReader::addToEntityReplacementDictionary(const std::string& strName, const std::string& strReplacementText)
{
    if (strName == "amp" ||
        strName == "lt" ||
        strName == "gt" ||
        strName == "apos" ||
        strName == "quot")
    {
        throw new std::invalid_argument("Redefinition of built-in entity.");
    }

    m_aEntityReplacementDictionary[strName] = strReplacementText;
    m_aREntityReplacementDictionary[std::string(strName.rbegin(), strName.rend())] = std::string(strReplacementText.rbegin(), strReplacementText.rend());
    return 0;
}

bool XMLEventReader::HandleTag()
{
    char cByte('\0');
    m_aStream.get(cByte);

    if (m_aStream.eof() == true)
    {
        throw new std::runtime_error("Tag incomplete.");
    }

    if (m_aStream.bad() == true)
    {
        throw new std::runtime_error("Stream is bad.");
    }

    if (cByte == '?')
    {
        if (HandleProcessingInstruction() == true)
        {
            return true;
        }
        else
        {
            m_bHasNextCalled = false;
            return hasNext();
        }
    }
    else if (cByte == '/')
    {
        return HandleTagEnd();
    }
    else if (cByte == '!')
    {
        return HandleMarkupDeclaration();
    }
    else if (std::isalpha(cByte, m_aLocale) == true ||
             cByte == '_')
    {
        return HandleTagStart(cByte);
    }
    else
    {
        int nByte(cByte);
        std::stringstream aMessage;
        aMessage << "Unknown byte '" << cByte << "' (0x"
                 << std::hex << std::uppercase << std::right << std::setfill('0') << std::setw(2) << nByte
                 << ") within element.";
        throw new std::runtime_error(aMessage.str());
    }
}

bool XMLEventReader::HandleTagStart(const char& cFirstByte)
{
    char cByte('\0');
    m_aStream.get(cByte);

    if (m_aStream.eof() == true)
    {
        throw new std::runtime_error("Tag start incomplete.");
    }

    if (m_aStream.bad() == true)
    {
        throw new std::runtime_error("Stream is bad.");
    }

    std::unique_ptr<std::string> pNamePrefix(nullptr);
    std::unique_ptr<std::string> pNameLocalPart(new std::string());
    std::unique_ptr<std::list<std::unique_ptr<Attribute>>> pAttributes(new std::list<std::unique_ptr<Attribute>>);

    pNameLocalPart->push_back(cFirstByte);

    do
    {
        if (cByte == ':')
        {
            if (pNamePrefix != nullptr)
            {
                throw new std::runtime_error("There can't be two prefixes in element name.");
            }

            pNamePrefix = std::move(pNameLocalPart);
            pNameLocalPart.reset(new std::string());
        }
        else if (cByte == '>')
        {
            if (pNamePrefix == nullptr)
            {
                pNamePrefix.reset(new std::string);
            }
            else
            {
                // if (pNamePrefix->length() <= 0) can't happen because cFirstByte
                // would already be a character for the prefix name. The prefix
                // name was already checked to contain valid characters.
            }

            std::unique_ptr<QName> pName(new QName("", *pNameLocalPart, *pNamePrefix));
            std::unique_ptr<StartElement> pStartElement(new StartElement(std::move(pName), std::move(pAttributes)));
            std::unique_ptr<XMLEvent> pEvent(new XMLEvent(std::move(pStartElement),
                                                          nullptr,
                                                          nullptr,
                                                          nullptr,
                                                          nullptr));
            m_aEvents.push(std::move(pEvent));
            break;
        }
        else if (cByte == '/')
        {
            m_aStream.get(cByte);

            if (m_aStream.eof() == true)
            {
                throw new std::runtime_error("Tag start incomplete.");
            }

            if (m_aStream.bad() == true)
            {
                throw new std::runtime_error("Stream is bad.");
            }

            if (cByte != '>')
            {
                throw new std::runtime_error("Empty start + end tag end without closing '>'.");
            }

            if (pNamePrefix == nullptr)
            {
                pNamePrefix.reset(new std::string);
            }
            else
            {
                // if (pNamePrefix->length() <= 0) can't happen because cFirstByte
                // would already be a character for the prefix name. The prefix
                // name was already checked to contain valid characters.
            }

            std::unique_ptr<QName> pName(new QName("", *pNameLocalPart, *pNamePrefix));
            std::unique_ptr<StartElement> pStartElement(new StartElement(std::move(pName), std::move(pAttributes)));
            std::unique_ptr<XMLEvent> pEvent(new XMLEvent(std::move(pStartElement),
                                                          nullptr,
                                                          nullptr,
                                                          nullptr,
                                                          nullptr));
            m_aEvents.push(std::move(pEvent));

            pName = std::unique_ptr<QName>(new QName("", *pNameLocalPart, *pNamePrefix));
            std::unique_ptr<EndElement> pEndElement(new EndElement(std::move(pName)));
            pEvent = std::unique_ptr<XMLEvent>(new XMLEvent(nullptr,
                                                            std::move(pEndElement),
                                                            nullptr,
                                                            nullptr,
                                                            nullptr));
            m_aEvents.push(std::move(pEvent));

            break;
        }
        else if (std::isspace(cByte, m_aLocale) != 0)
        {
            if (pNameLocalPart->length() <= 0)
            {
                throw new std::runtime_error("Start tag name begins with whitespace.");
            }

            while (true)
            {
                m_aStream.get(cByte);

                if (m_aStream.eof() == true)
                {
                    throw new std::runtime_error("Tag start incomplete.");
                }

                if (m_aStream.bad() == true)
                {
                    throw new std::runtime_error("Stream is bad.");
                }

                if (cByte == '>')
                {
                    break;
                }
                else if (std::isspace(cByte, m_aLocale) != 0)
                {
                    // Ignore/consume.
                }
                else if (cByte == '/')
                {
                    m_aStream.unget();

                    if (m_aStream.bad() == true)
                    {
                        throw new std::runtime_error("Stream is bad.");
                    }

                    break;
                }
                else
                {
                    HandleAttributes(cByte, pAttributes);
                    break;
                }
            }
        }
        else if (std::isalnum(cByte, m_aLocale) == true ||
                 cByte == '-' ||
                 cByte == '_' ||
                 cByte == '.')
        {
            pNameLocalPart->push_back(cByte);
        }
        else
        {
            int nByte(cByte);
            std::stringstream aMessage;
            aMessage << "Character '" << cByte << "' (0x"
                     << std::hex << std::uppercase << std::right << std::setfill('0') << std::setw(2) << nByte
                     << ") not supported in a start tag name.";
            throw new std::runtime_error(aMessage.str());
        }

        m_aStream.get(cByte);

        if (m_aStream.eof() == true)
        {
            throw new std::runtime_error("Tag start incomplete.");
        }

        if (m_aStream.bad() == true)
        {
            throw new std::runtime_error("Stream is bad.");
        }

    } while (true);

    return true;
}

bool XMLEventReader::HandleTagEnd()
{
    char cByte('\0');
    m_aStream.get(cByte);

    if (m_aStream.eof() == true)
    {
        throw new std::runtime_error("Tag end incomplete.");
    }

    if (m_aStream.bad() == true)
    {
        throw new std::runtime_error("Stream is bad.");
    }

    std::unique_ptr<std::string> pNamePrefix(nullptr);
    std::unique_ptr<std::string> pNameLocalPart(new std::string());

    // No validity check for the XML element name is needed
    // if end tags are compared to start tags and the start
    // tags were already checked.

    do
    {
        if (cByte == ':')
        {
            if (pNamePrefix != nullptr)
            {
                throw new std::runtime_error("There can't be two prefixes in the element name.");
            }

            pNamePrefix = std::move(pNameLocalPart);
            pNameLocalPart.reset(new std::string());
        }
        else if (cByte == '>')
        {
            if (pNamePrefix == nullptr)
            {
                pNamePrefix.reset(new std::string);
            }
            else
            {
                // if (pNamePrefix->length() <= 0) will result in an empty prefix
                // of the QName anyway, but the reader could complain as well.
                // The prefix name was already checked to contain valid characters.
            }

            std::unique_ptr<QName> pName(new QName("", *pNameLocalPart, *pNamePrefix));
            std::unique_ptr<EndElement> pEndElement(new EndElement(std::move(pName)));
            std::unique_ptr<XMLEvent> pEvent(new XMLEvent(nullptr,
                                                          std::move(pEndElement),
                                                          nullptr,
                                                          nullptr,
                                                          nullptr));
            m_aEvents.push(std::move(pEvent));
            return true;
        }
        else if (std::isalnum(cByte, m_aLocale) == true ||
                 cByte == '-' ||
                 cByte == '_' ||
                 cByte == '.')
        {
            pNameLocalPart->push_back(cByte);
        }
        else
        {
            int nByte(cByte);
            std::stringstream aMessage;
            aMessage << "Character '" << cByte << "' (0x"
                     << std::hex << std::uppercase << std::right << std::setfill('0') << std::setw(2) << nByte
                     << ") not supported in an end tag name.";
            throw new std::runtime_error(aMessage.str());
        }

        m_aStream.get(cByte);

        if (m_aStream.eof() == true)
        {
            throw new std::runtime_error("End tag incomplete.");
        }

        if (m_aStream.bad() == true)
        {
            throw new std::runtime_error("Stream is bad.");
        }

    } while (true);
}

bool XMLEventReader::HandleText(const char& cFirstByte)
{
    std::unique_ptr<std::string> pData(new std::string);

    if (cFirstByte == '&')
    {
        std::unique_ptr<std::string> pResolvedText(nullptr);

        ResolveEntity(pResolvedText);
        pData->append(*pResolvedText);
    }
    else
    {
        pData->push_back(cFirstByte);
    }

    char cByte('\0');

    while (true)
    {
        m_aStream.get(cByte);

        if (m_aStream.eof() == true)
        {
            break;
        }

        if (m_aStream.bad() == true)
        {
            throw new std::runtime_error("Stream is bad.");
        }

        if (cByte == '<')
        {
            m_aStream.unget();

            if (m_aStream.bad() == true)
            {
                throw new std::runtime_error("Stream is bad.");
            }

            break;
        }
        else if (cByte == '&')
        {
            std::unique_ptr<std::string> pResolvedText(nullptr);

            ResolveEntity(pResolvedText);
            pData->append(*pResolvedText);
        }
        else
        {
            pData->push_back(cByte);
        }
    }

    std::unique_ptr<Characters> pCharacters(new Characters(std::move(pData)));
    std::unique_ptr<XMLEvent> pEvent(new XMLEvent(nullptr,
                                                  nullptr,
                                                  std::move(pCharacters),
                                                  nullptr,
                                                  nullptr));
    m_aEvents.push(std::move(pEvent));

    return true;
}

bool XMLEventReader::HandleProcessingInstruction()
{
    std::unique_ptr<std::string> pTarget(nullptr);

    HandleProcessingInstructionTarget(pTarget);

    if (pTarget == nullptr)
    {
        throw new std::runtime_error("Processing instruction without target name.");
    }

    if (pTarget->length() == 3)
    {
        if ((pTarget->at(0) == 'x' ||
             pTarget->at(0) == 'X') &&
            (pTarget->at(1) == 'm' ||
             pTarget->at(1) == 'M') &&
            (pTarget->at(2) == 'l' ||
             pTarget->at(2) == 'L'))
        {
            /** @todo This should read the XML declaration instructions instead
              * of just consuming/ignoring it. */

            char cByte('\0');
            int nMatchCount = 0;

            while (nMatchCount < 2)
            {
                m_aStream.get(cByte);

                if (m_aStream.eof() == true)
                {
                    throw new std::runtime_error("XML declaration incomplete.");
                }

                if (m_aStream.bad() == true)
                {
                    throw new std::runtime_error("Stream is bad.");
                }

                if (cByte == '?' &&
                    nMatchCount <= 0)
                {
                    nMatchCount++;
                }
                else if (cByte == '>' &&
                        nMatchCount <= 1)
                {
                    return false;
                }
            }

            return false;
        }
    }

    std::unique_ptr<std::string> pData(new std::string);
    char cByte('\0');
    int nMatchCount = 0;

    while (nMatchCount < 2)
    {
        m_aStream.get(cByte);

        if (m_aStream.eof() == true)
        {
            throw new std::runtime_error("Processing instruction data incomplete.");
        }

        if (m_aStream.bad() == true)
        {
            throw new std::runtime_error("Stream is bad.");
        }

        if (cByte == '?' &&
            nMatchCount <= 0)
        {
            nMatchCount++;
        }
        else if (cByte == '>' &&
                 nMatchCount <= 1)
        {
            //nMatchCount++;

            std::unique_ptr<ProcessingInstruction> pProcessingInstruction(new ProcessingInstruction(std::move(pTarget), std::move(pData)));
            std::unique_ptr<XMLEvent> pEvent(new XMLEvent(nullptr,
                                                          nullptr,
                                                          nullptr,
                                                          nullptr,
                                                          std::move(pProcessingInstruction)));
            m_aEvents.push(std::move(pEvent));

            return true;
        }
        else
        {
            if (nMatchCount > 0)
            {
                pData->push_back('?');
            }

            nMatchCount = 0;

            pData->push_back(cByte);
        }
    }

    return false;
}

bool XMLEventReader::HandleProcessingInstructionTarget(std::unique_ptr<std::string>& pTarget)
{
    std::unique_ptr<std::string> pName(nullptr);
    char cByte('\0');
    int nMatchCount = 0;

    while (nMatchCount < 2)
    {
        m_aStream.get(cByte);

        if (m_aStream.eof() == true)
        {
            throw new std::runtime_error("Processing instruction target name incomplete.");
        }

        if (m_aStream.bad() == true)
        {
            throw new std::runtime_error("Stream is bad.");
        }

        if (cByte == '?' &&
            nMatchCount <= 0)
        {
            nMatchCount++;
        }
        else if (cByte == '>' &&
                 nMatchCount <= 1)
        {
            throw new std::runtime_error("Processing instruction ended before processing instruction target name could be read.");
        }
        else if (std::isspace(cByte, m_aLocale) != 0)
        {
            if (pName == nullptr)
            {
                throw new std::runtime_error("Processing instruction without target name.");
            }

            pTarget = std::move(pName);

            return true;
        }
        else
        {
            if (nMatchCount > 0)
            {
                throw new std::runtime_error("Processing instruction target name interrupted by '?'.");
            }

            if (pName == nullptr)
            {
                if (std::isalpha(cByte, m_aLocale) != true)
                {
                    int nByte(cByte);
                    std::stringstream aMessage;
                    aMessage << "Character '" << cByte << "' (0x"
                             << std::hex << std::uppercase << std::right << std::setfill('0') << std::setw(2) << nByte
                             << ") not supported as first character of an processing instruction target name.";
                    throw new std::runtime_error(aMessage.str());
                }

                pName = std::unique_ptr<std::string>(new std::string);
            }

            pName->push_back(cByte);
        }
    }

    return false;
}

bool XMLEventReader::HandleMarkupDeclaration()
{
    char cByte('\0');
    m_aStream.get(cByte);

    if (m_aStream.eof() == true)
    {
        throw new std::runtime_error("Markup declaration incomplete.");
    }

    if (m_aStream.bad() == true)
    {
        throw new std::runtime_error("Steam is bad.");
    }

    if (cByte == '-')
    {
        return HandleComment();
    }
    else
    {
        throw new std::runtime_error("Markup declaration type not implemented yet.");
    }

    return true;
}

bool XMLEventReader::HandleComment()
{
    char cByte('\0');
    m_aStream.get(cByte);

    if (m_aStream.eof() == true)
    {
        throw new std::runtime_error("Comment incomplete.");
    }

    if (m_aStream.bad() == true)
    {
        throw new std::runtime_error("Stream is bad.");
    }

    if (cByte != '-')
    {
        throw new std::runtime_error("Comment malformed.");
    }

    std::unique_ptr<std::string> pData(new std::string);

    unsigned int nMatchCount = 0;
    const unsigned int END_SEQUENCE_LENGTH = 3;
    char cEndSequence[END_SEQUENCE_LENGTH] = { '-', '-', '>' };

    do
    {
        m_aStream.get(cByte);

        if (m_aStream.eof() == true)
        {
            throw new std::runtime_error("Comment incomplete.");
        }

        if (m_aStream.bad() == true)
        {
            throw new std::runtime_error("Stream is bad.");
        }

        if (cByte == cEndSequence[nMatchCount])
        {
            if (nMatchCount + 1 < END_SEQUENCE_LENGTH)
            {
                ++nMatchCount;
            }
            else
            {
                std::unique_ptr<Comment> pComment(new Comment(std::move(pData)));
                std::unique_ptr<XMLEvent> pEvent(new XMLEvent(nullptr,
                                                              nullptr,
                                                              nullptr,
                                                              std::move(pComment),
                                                              nullptr));
                m_aEvents.push(std::move(pEvent));

                break;
            }
        }
        else
        {
            if (nMatchCount > 0)
            {
                // Instead of strncpy() and C-style char*.
                for (unsigned int i = 0; i < nMatchCount; i++)
                {
                    pData->push_back(cEndSequence[i]);
                }

                pData->push_back(cByte);
                nMatchCount = 0;
            }
            else
            {
                pData->push_back(cByte);
            }
        }

    } while (true);

    return true;
}

bool XMLEventReader::HandleAttributes(const char& cFirstByte, std::unique_ptr<std::list<std::unique_ptr<Attribute>>>& pAttributes)
{
    if (pAttributes == nullptr)
    {
        throw new std::invalid_argument("nullptr passed.");
    }

    std::unique_ptr<QName> pAttributeName(nullptr);
    std::unique_ptr<std::string> pAttributeValue(nullptr);

    HandleAttributeName(cFirstByte, pAttributeName);
    HandleAttributeValue(pAttributeValue);

    pAttributes->push_back(std::unique_ptr<Attribute>(new Attribute(std::move(pAttributeName), std::move(pAttributeValue))));

    char cByte('\0');

    do
    {
        m_aStream.get(cByte);

        if (m_aStream.eof() == true)
        {
            throw new std::runtime_error("Tag start incomplete.");
        }

        if (m_aStream.bad() == true)
        {
            throw new std::runtime_error("Stream is bad.");
        }

        if (cByte == '>')
        {
            // Not part of the attributes any more and indicator for outer
            // methods to complete the StartElement.
            m_aStream.unget();
            break;
        }
        else if (cByte == '/')
        {
            m_aStream.get(cByte);

            if (m_aStream.eof() == true)
            {
                throw new std::runtime_error("Tag start incomplete.");
            }

            if (m_aStream.bad() == true)
            {
                throw new std::runtime_error("Stream is bad.");
            }

            if (cByte != '>')
            {
                throw new std::runtime_error("Empty start + end tag end without closing '>'.");
            }

            m_aStream.unget();

            if (m_aStream.bad() == true)
            {
                throw new std::runtime_error("Stream is bad.");
            }

            m_aStream.unget();

            if (m_aStream.bad() == true)
            {
                throw new std::runtime_error("Stream is bad.");
            }

            break;
        }
        else if (std::isspace(cByte, m_aLocale) != 0)
        {
            // Ignore/consume.
            continue;
        }
        else
        {
            HandleAttributeName(cByte, pAttributeName);
            HandleAttributeValue(pAttributeValue);

            pAttributes->push_back(std::unique_ptr<Attribute>(new Attribute(std::move(pAttributeName), std::move(pAttributeValue))));
        }

    } while (true);

    return true;
}

bool XMLEventReader::HandleAttributeName(const char& cFirstByte, std::unique_ptr<QName>& pName)
{
    std::unique_ptr<std::string> pNamePrefix(nullptr);
    std::unique_ptr<std::string> pNameLocalPart(new std::string());

    if (std::isalnum(cFirstByte, m_aLocale) == true ||
        cFirstByte == '_')
    {
        pNameLocalPart->push_back(cFirstByte);
    }
    else
    {
        int nByte(cFirstByte);
        std::stringstream aMessage;
        aMessage << "Character '" << cFirstByte << "' (0x"
                 << std::hex << std::uppercase << std::right << std::setfill('0') << std::setw(2) << nByte
                 << ") not supported as first character of an attribute name.";
        throw new std::runtime_error(aMessage.str());
    }

    char cByte('\0');

    do
    {
        m_aStream.get(cByte);

        if (m_aStream.eof() == true)
        {
            throw new std::runtime_error("Attribute name incomplete.");
        }

        if (m_aStream.bad() == true)
        {
            throw new std::runtime_error("Stream is bad.");
        }

        if (cByte == ':')
        {
            if (pNamePrefix != nullptr)
            {
                throw new std::runtime_error("There can't be two prefixes in attribute name.");
            }

            pNamePrefix = std::move(pNameLocalPart);
            pNameLocalPart.reset(new std::string());
        }
        else if (std::isspace(cByte, m_aLocale) != 0)
        {
            cByte = ConsumeWhitespace();

            if (cByte == '\0')
            {
                throw new std::runtime_error("Attribute incomplete.");
            }
            else if (cByte != '=')
            {
                throw new std::runtime_error("Attribute name is malformed.");
            }

            // To make sure that the next loop iteration will end up in cByte == '='.
            m_aStream.unget();

            if (m_aStream.bad() == true)
            {
                throw new std::runtime_error("Stream is bad.");
            }
        }
        else if (cByte == '=')
        {
            if (pNamePrefix == nullptr)
            {
                pNamePrefix.reset(new std::string);
            }
            else
            {
                // if (pNamePrefix->length() <= 0) can't happen because cFirstByte
                // would already be a character for the prefix name. The prefix
                // name was already checked to contain valid characters.
            }

            pName = std::unique_ptr<QName>(new QName("", *pNameLocalPart, *pNamePrefix));

            return true;
        }
        else if (std::isalnum(cByte, m_aLocale) == true ||
                 cByte == '-' ||
                 cByte == '_' ||
                 cByte == '.')
        {
            pNameLocalPart->push_back(cByte);
        }
        else
        {
            int nByte(cByte);
            std::stringstream aMessage;
            aMessage << "Character '" << cByte << "' (0x"
                     << std::hex << std::uppercase << std::right << std::setfill('0') << std::setw(2) << nByte
                     << ") not supported in an attribute name.";
            throw new std::runtime_error(aMessage.str());
        }

    } while (true);

    return false;
}

bool XMLEventReader::HandleAttributeValue(std::unique_ptr<std::string>& pValue)
{
    pValue = std::unique_ptr<std::string>(new std::string);
    char cDelimiter(ConsumeWhitespace());

    if (cDelimiter == '\0')
    {
        throw new std::runtime_error("Attribute is missing its value.");
    }
    else if (cDelimiter != '\'' &&
             cDelimiter != '"')
    {
        int nByte(cDelimiter);
        std::stringstream aMessage;
        aMessage << "Attribute value doesn't start with a delimiter like ''' or '\"', instead, '" << cDelimiter << "' (0x"
                 << std::hex << std::uppercase << std::right << std::setfill('0') << std::setw(2) << nByte
                 << ") was found.";
        throw new std::runtime_error(aMessage.str());
    }

    char cByte('\0');

    do
    {
        m_aStream.get(cByte);

        if (m_aStream.eof() == true)
        {
            throw new std::runtime_error("Attribute value incomplete.");
        }

        if (m_aStream.bad() == true)
        {
            throw new std::runtime_error("Stream is bad.");
        }

        if (cByte == cDelimiter)
        {
            return true;
        }
        else if (cByte == '&')
        {
            std::unique_ptr<std::string> pResolvedText(nullptr);

            ResolveEntity(pResolvedText);
            pValue->append(*pResolvedText);
        }
        else
        {
            pValue->push_back(cByte);
        }

    } while (true);

    return false;
}

/**
 * @todo This may or may not be adjusted to become a little more like
 *     XMLEventReader::ResolveREntity(), so that no special handling of
 *     the first m_aStream.get() is needed outside of the loop.
 */
void XMLEventReader::ResolveEntity(std::unique_ptr<std::string>& pResolvedText)
{
    if (pResolvedText != nullptr)
    {
        throw new std::invalid_argument("No nullptr passed.");
    }

    char cByte('\0');
    m_aStream.get(cByte);

    if (m_aStream.eof() == true)
    {
        throw new std::runtime_error("Entity incomplete.");
    }

    if (m_aStream.bad() == true)
    {
        throw new std::runtime_error("Stream is bad.");
    }

    if (cByte == ';')
    {
        throw new std::runtime_error("Entity has no name.");
    }
    else
    {
        std::unique_ptr<std::string> pEntityName(new std::string);
        pEntityName->push_back(cByte);

        do
        {
            m_aStream.get(cByte);

            if (m_aStream.eof() == true)
            {
                throw new std::runtime_error("Entity incomplete.");
            }

            if (m_aStream.bad() == true)
            {
                throw new std::runtime_error("Stream is bad.");
            }

            if (cByte == ';')
            {
                break;
            }

            pEntityName->push_back(cByte);

        } while (true);

        std::map<std::string, std::string>::iterator iter = m_aEntityReplacementDictionary.find(*pEntityName);

        if (iter != m_aEntityReplacementDictionary.end())
        {
            pResolvedText = std::unique_ptr<std::string>(new std::string(iter->second));
        }
        else
        {
            std::stringstream aMessage;
            aMessage << "Unable to resolve entity '&" << *pEntityName << ";'.";
            throw new std::runtime_error(aMessage.str());
        }
    }
}

/**
 * @retval Returns the first non-whitespace character or '\0' in
 *     case of end-of-file.
 */
char XMLEventReader::ConsumeWhitespace()
{
    char cByte('\0');

    do
    {
        m_aStream.get(cByte);

        if (m_aStream.eof() == true)
        {
            return '\0';
        }

        if (m_aStream.bad() == true)
        {
            throw new std::runtime_error("Stream is bad.");
        }

        if (std::isspace(cByte, m_aLocale) == 0)
        {
            return cByte;
        }

    } while (true);
}

bool XMLEventReader::HandleRTag()
{
    char cByte('\0');

    if (rget(cByte) != true)
    {
        throw new std::runtime_error("Tag incomplete.");
    }

    if (cByte == '?')
    {
        if (HandleRProcessingInstruction() == true)
        {
            return true;
        }
        else
        {
            m_bHasPreviousCalled = false;
            return hasPrevious();
        }
    }
    else if (std::isspace(cByte) != 0)
    {
        cByte = ConsumeRWhitespace();

        if (cByte == '\0')
        {
            throw new std::runtime_error("Start tag incomplete.");
        }
        else if (cByte != '"' &&
                 cByte != '\'')
        {
            int nByte(cByte);
            std::stringstream aMessage;
            aMessage << "Attribute value in start element doesn't start with a delimiter like ''' or '\"', instead, '"
                     << cByte << "' (0x"
                     << std::hex << std::uppercase << std::right << std::setfill('0') << std::setw(2) << nByte
                     << ") was found.";
            throw new std::runtime_error(aMessage.str());
        }

        std::unique_ptr<StartElement> pStartElement(nullptr);

        if (HandleRTagStart(cByte, pStartElement) == true)
        {
            std::unique_ptr<XMLEvent> pEvent(new XMLEvent(std::move(pStartElement),
                                                          nullptr,
                                                          nullptr,
                                                          nullptr,
                                                          nullptr));
            m_aEvents.push(std::move(pEvent));
            return true;
        }
        else
        {
            return false;
        }
    }
    else if (cByte == '/')
    {
        cByte = ConsumeRWhitespace();

        if (cByte == '\0')
        {
            throw new std::runtime_error("Start tag incomplete.");
        }
        else if (cByte != '"' &&
                 cByte != '\'')
        {
            int nByte(cByte);
            std::stringstream aMessage;
            aMessage << "Attribute value doesn't start with a delimiter like ''' or '\"', instead, '"
                     << cByte << "' (0x"
                     << std::hex << std::uppercase << std::right << std::setfill('0') << std::setw(2) << nByte
                     << ") was found.";
            throw new std::runtime_error(aMessage.str());
        }

        std::unique_ptr<StartElement> pStartElement(nullptr);

        if (HandleRTagStart(cByte, pStartElement) == true)
        {
            std::unique_ptr<QName> pName(new QName(pStartElement->getName().getNamespaceURI(),
                                                   pStartElement->getName().getLocalPart(),
                                                   pStartElement->getName().getPrefix()));
            std::unique_ptr<EndElement> pEndElement(new EndElement(std::move(pName)));
            std::unique_ptr<XMLEvent> pEvent(new XMLEvent(nullptr,
                                                          std::move(pEndElement),
                                                          nullptr,
                                                          nullptr,
                                                          nullptr));
            m_aEvents.push(std::move(pEvent));

            pEvent.reset(new XMLEvent(std::move(pStartElement),
                                      nullptr,
                                      nullptr,
                                      nullptr,
                                      nullptr));
            m_aEvents.push(std::move(pEvent));

            return true;
        }
        else
        {
            return false;
        }
    }
    else if (cByte == '"' ||
             cByte == '\'')
    {
        std::unique_ptr<StartElement> pStartElement(nullptr);

        if (HandleRTagStart(cByte, pStartElement) == true)
        {
            std::unique_ptr<XMLEvent> pEvent(new XMLEvent(std::move(pStartElement),
                                                          nullptr,
                                                          nullptr,
                                                          nullptr,
                                                          nullptr));
            m_aEvents.push(std::move(pEvent));
            return true;
        }
        else
        {
            return false;
        }
    }
    else if (cByte == '-')
    {
        if (rget(cByte) != true)
        {
            throw new std::runtime_error("Tag incomplete.");
        }

        if (cByte == '-')
        {
            return HandleRComment();
        }
        else
        {
            runget();
            return HandleRTagStartEnd('-');
        }
    }
    else
    {
        return HandleRTagStartEnd(cByte);
    }
}

bool XMLEventReader::HandleRTagStartEnd(const char& cFirstByte)
{
    std::unique_ptr<QName> pName(nullptr);

    if (HandleRTagName(cFirstByte, pName) != true)
    {
        throw new std::runtime_error("Tag name incomplete.");
    }

    char cByte('\0');

    if (rget(cByte) != true)
    {
        throw new std::runtime_error("Tag incomplete.");
    }

    if (cByte == '<')
    {
        std::unique_ptr<std::list<std::unique_ptr<Attribute>>> pAttributes(new std::list<std::unique_ptr<Attribute>>);
        std::unique_ptr<StartElement> pStartElement(new StartElement(std::move(pName), std::move(pAttributes)));
        std::unique_ptr<XMLEvent> pEvent(new XMLEvent(std::move(pStartElement),
                                                      nullptr,
                                                      nullptr,
                                                      nullptr,
                                                      nullptr));
        m_aEvents.push(std::move(pEvent));
        return true;
    }
    else if (cByte == '/')
    {
        if (rget(cByte) != true)
        {
            throw new std::runtime_error("End tag incomplete.");
        }

        if (cByte != '<')
        {
            throw new std::runtime_error("End tag incomplete.");
        }

        std::unique_ptr<EndElement> pEndElement(new EndElement(std::move(pName)));
        std::unique_ptr<XMLEvent> pEvent(new XMLEvent(nullptr,
                                                      std::move(pEndElement),
                                                      nullptr,
                                                      nullptr,
                                                      nullptr));
        m_aEvents.push(std::move(pEvent));
        return true;
    }
    else
    {
        int nByte(cByte);
        std::stringstream aMessage;
        aMessage << "Character '" << cByte << "' (0x"
                 << std::hex << std::uppercase << std::right << std::setfill('0') << std::setw(2) << nByte
                 << ") not supported in a tag.";
        throw new std::runtime_error(aMessage.str());
    }
}

bool XMLEventReader::HandleRTagStart(char cByte, std::unique_ptr<StartElement>& pStartElement)
{
    std::unique_ptr<std::list<std::unique_ptr<Attribute>>> pAttributes(new std::list<std::unique_ptr<Attribute>>);

    if (cByte == '"' ||
        cByte == '\'')
    {
        HandleRAttributes(cByte, pAttributes);

        if (rget(cByte) != true)
        {
            throw new std::runtime_error("Start tag incomplete.");
        }
    }

    std::unique_ptr<QName> pName(nullptr);

    if (HandleRTagName(cByte, pName) != true)
    {
        throw new std::runtime_error("Tag name incomplete.");
    }

    if (rget(cByte) != true)
    {
        throw new std::runtime_error("Start tag incomplete.");
    }

    if (cByte != '<')
    {
        throw new std::runtime_error("Start tag incomplete.");
    }

    pStartElement.reset(new StartElement(std::move(pName), std::move(pAttributes)));

    return true;
}

bool XMLEventReader::HandleRTagName(const char& cFirstByte, std::unique_ptr<QName>& pName)
{
    if (std::isalnum(cFirstByte, m_aLocale) != true &&
        cFirstByte != '-' &&
        cFirstByte != '_' &&
        cFirstByte != '.')
    {
        int nByte(cFirstByte);
        std::stringstream aMessage;
        aMessage << "Unknown byte '" << cFirstByte << "' (0x"
                 << std::hex << std::uppercase << std::right << std::setfill('0') << std::setw(2) << nByte
                 << ") within element name.";
        throw new std::runtime_error(aMessage.str());
    }

    std::unique_ptr<std::string> pNamePrefix(nullptr);
    std::unique_ptr<std::string> pNameLocalPart(new std::string());

    pNameLocalPart->push_back(cFirstByte);

    char cByte('\0');

    do
    {
        if (rget(cByte) != true)
        {
            throw new std::runtime_error("Tag name incomplete.");
        }

        if (cByte == '<' ||
            cByte == '/')
        {
            cByte = pNameLocalPart->back();

            if (std::isalnum(cByte, m_aLocale) != true &&
                cByte != '_')
            {
                throw new std::runtime_error("Tag name malformed.");
            }

            if (pNamePrefix == nullptr)
            {
                pNamePrefix.reset(new std::string);
            }
            else
            {
                // if (pNamePrefix->length() <= 0) will result in an empty prefix
                // of the QName anyway, but the reader could complain as well.

                std::reverse(pNamePrefix->begin(), pNamePrefix->end());

                if (pNamePrefix->length() > 0)
                {
                    // Other characters were already checked.
                    cByte = pNamePrefix->at(0);

                    if (std::isalnum(cByte, m_aLocale) != true &&
                        cByte != '_')
                    {
                        int nByte(cByte);
                        std::stringstream aMessage;
                        aMessage << "Character '" << cByte << "' (0x"
                                 << std::hex << std::uppercase << std::right << std::setfill('0') << std::setw(2) << nByte
                                 << ") not supported as first character of a prefix name.";
                        throw new std::runtime_error(aMessage.str());
                    }
                }
            }

            std::reverse(pNameLocalPart->begin(), pNameLocalPart->end());

            pName.reset(new QName("", *pNameLocalPart, *pNamePrefix));

            runget();

            return true;
        }
        else if (std::isalnum(cByte, m_aLocale) == true ||
                 cByte == '-' ||
                 cByte == '_' ||
                 cByte == '.')
        {
            if (pNamePrefix != nullptr)
            {
                pNamePrefix->push_back(cByte);
            }
            else
            {
                pNameLocalPart->push_back(cByte);
            }
        }
        else if (cByte == ':')
        {
            if (pNamePrefix != nullptr)
            {
                throw new std::runtime_error("There can't be two prefixes in element name.");
            }

            pNamePrefix.reset(new std::string());
        }
        else
        {
            int nByte(cByte);
            std::stringstream aMessage;
            aMessage << "Character '" << cByte << "' (0x"
                     << std::hex << std::uppercase << std::right << std::setfill('0') << std::setw(2) << nByte
                     << ") not supported in element name.";
            throw new std::runtime_error(aMessage.str());
        }

    } while (true);
}

bool XMLEventReader::HandleRText(const char& cFirstByte)
{
    std::unique_ptr<std::string> pData(new std::string);

    if (cFirstByte == ';')
    {
        std::unique_ptr<std::string> pResolvedText(nullptr);

        ResolveREntity(pResolvedText);
        pData->append(*pResolvedText);
    }
    else
    {
        pData->push_back(cFirstByte);
    }

    char cByte('\0');

    while (true)
    {
        if (rget(cByte) != true)
        {
            break;
        }

        if (cByte == '>')
        {
            runget();
            break;
        }
        // '<' and '&' are illegal here, but not a breaking issue when reading
        // backwards/reverse, and as we're not checking well-formedness yet,
        // they're handled as normal characters for now.
        else if (cByte == ';')
        {
            std::unique_ptr<std::string> pResolvedText(nullptr);

            ResolveREntity(pResolvedText);
            pData->append(*pResolvedText);
        }
        else
        {
            pData->push_back(cByte);
        }
    }

    std::reverse(pData->begin(), pData->end());

    std::unique_ptr<Characters> pCharacters(new Characters(std::move(pData)));
    std::unique_ptr<XMLEvent> pEvent(new XMLEvent(nullptr,
                                                  nullptr,
                                                  std::move(pCharacters),
                                                  nullptr,
                                                  nullptr));
    m_aEvents.push(std::move(pEvent));

    return true;
}

bool XMLEventReader::HandleRProcessingInstruction()
{
    std::unique_ptr<std::string> pData(new std::string);
    char cByte('\0');
    int nMatchCount = 0;
    int nTargetCount = 0;
    int nSpaceCount = 0;

    while (nMatchCount < 2)
    {
        if (rget(cByte) != true)
        {
            throw new std::runtime_error("Processing instruction target or data incomplete.");
        }

        if (std::isspace(cByte, m_aLocale) != 0)
        {
            pData->push_back(cByte);

            if (nTargetCount > 0)
            {
                nSpaceCount = 1;
            }
            else
            {
                ++nSpaceCount;
            }

            nTargetCount = 0;
        }
        else
        {
            if (cByte == '?' &&
                nMatchCount <= 0)
            {
                nMatchCount++;
            }
            else if (cByte == '<' &&
                     nMatchCount <= 1)
            {
                //nMatchCount++;

                if (pData->length() <= 0)
                {
                    throw new std::runtime_error("Processing instruction ended before processing instruction target name could be read.");
                }

                std::reverse(pData->begin(), pData->end());

                if (nTargetCount <= 0)
                {
                    throw new std::runtime_error("Processing instruction without target name.");
                }

                std::unique_ptr<std::string> pTarget(new std::string(pData->substr(0, nTargetCount)));

                if (pTarget->length() == 3)
                {
                    if ((pTarget->at(0) == 'x' ||
                         pTarget->at(0) == 'X') &&
                        (pTarget->at(1) == 'm' ||
                         pTarget->at(1) == 'M') &&
                        (pTarget->at(2) == 'l' ||
                         pTarget->at(2) == 'L'))
                    {
                        /** @todo This should read the XML declaration instructions instead
                          * of just consuming/ignoring it. */

                        return false;
                    }
                }

                if (std::isalpha(pTarget->at(0), m_aLocale) != true)
                {
                    int nByte(pTarget->at(0));
                    std::stringstream aMessage;
                    aMessage << "Character '" << pTarget->at(0) << "' (0x"
                             << std::hex << std::uppercase << std::right << std::setfill('0') << std::setw(2) << nByte
                             << ") not supported as first character of an processing instruction target name.";
                    throw new std::runtime_error(aMessage.str());
                }

                pData->erase(0, nTargetCount + nSpaceCount);

                std::unique_ptr<ProcessingInstruction> pProcessingInstruction(new ProcessingInstruction(std::move(pTarget), std::move(pData)));
                std::unique_ptr<XMLEvent> pEvent(new XMLEvent(nullptr,
                                                              nullptr,
                                                              nullptr,
                                                              nullptr,
                                                              std::move(pProcessingInstruction)));
                m_aEvents.push(std::move(pEvent));

                return true;
            }
            else
            {
                if (nMatchCount > 0)
                {
                    throw new std::runtime_error("Processing instruction target name interrupted by '?'.");
                }

                nMatchCount = 0;
                ++nTargetCount;

                pData->push_back(cByte);
            }
        }
    }

    return false;
}

bool XMLEventReader::HandleRComment()
{
    std::unique_ptr<std::string> pData(new std::string);

    unsigned int nMatchCount = 0;
    const unsigned int END_SEQUENCE_LENGTH = 4;
    char cEndSequence[END_SEQUENCE_LENGTH] = { '-', '-', '!', '<' };
    char cByte('\0');

    do
    {
        if (rget(cByte) != true)
        {
            throw new std::runtime_error("Comment incomplete.");
        }

        if (cByte == cEndSequence[nMatchCount])
        {
            if (nMatchCount + 1 < END_SEQUENCE_LENGTH)
            {
                ++nMatchCount;
            }
            else
            {
                std::reverse(pData->begin(), pData->end());

                std::unique_ptr<Comment> pComment(new Comment(std::move(pData)));
                std::unique_ptr<XMLEvent> pEvent(new XMLEvent(nullptr,
                                                              nullptr,
                                                              nullptr,
                                                              std::move(pComment),
                                                              nullptr));
                m_aEvents.push(std::move(pEvent));

                break;
            }
        }
        else
        {
            if (nMatchCount > 0)
            {
                // Instead of strncpy() and C-style char*.
                for (unsigned int i = 0; i < nMatchCount; i++)
                {
                    pData->push_back(cEndSequence[i]);
                }

                pData->push_back(cByte);
                nMatchCount = 0;
            }
            else
            {
                pData->push_back(cByte);
            }
        }

    } while (true);

    return true;
}

bool XMLEventReader::HandleRAttributes(const char& cFirstByte, std::unique_ptr<std::list<std::unique_ptr<Attribute>>>& pAttributes)
{
    if (pAttributes == nullptr)
    {
        throw new std::invalid_argument("nullptr passed.");
    }

    std::unique_ptr<std::string> pAttributeValue(nullptr);
    std::unique_ptr<QName> pAttributeName(nullptr);

    HandleRAttributeValue(cFirstByte, pAttributeValue);
    HandleRAttributeName(pAttributeName);

    pAttributes->push_back(std::unique_ptr<Attribute>(new Attribute(std::move(pAttributeName), std::move(pAttributeValue))));

    char cByte('\0');

    do
    {
        if (rget(cByte) != true)
        {
            throw new std::runtime_error("Attributes incomplete.");
        }

        if (std::isspace(cByte, m_aLocale) != 0)
        {
            // Ignore/consume. Should not be there because
            // HandleRAttributeName() already consumed whitespace.
            continue;
        }
        else if (cByte == '"' ||
                 cByte == '\'')
        {
            HandleRAttributeValue(cByte, pAttributeValue);
            HandleRAttributeName(pAttributeName);

            pAttributes->push_back(std::unique_ptr<Attribute>(new Attribute(std::move(pAttributeName), std::move(pAttributeValue))));
        }
        else
        {
            runget();
            break;
        }

    } while (true);

    return true;
}

bool XMLEventReader::HandleRAttributeValue(const char& cDelimiter, std::unique_ptr<std::string>& pValue)
{
    pValue = std::unique_ptr<std::string>(new std::string);
    char cByte('\0');

    do
    {
        if (rget(cByte) != true)
        {
            throw new std::runtime_error("Attribute value incomplete.");
        }

        if (cByte == cDelimiter)
        {
            cByte = ConsumeRWhitespace();

            if (cByte == '\0')
            {
                throw new std::runtime_error("Attribute incomplete.");
            }
            else if (cByte != '=')
            {
                throw new std::runtime_error("Attribute value is malformed.");
            }

            std::reverse(pValue->begin(), pValue->end());

            return true;
        }
        else if (cByte == ';')
        {
            std::unique_ptr<std::string> pResolvedText(nullptr);

            ResolveREntity(cDelimiter, pResolvedText);
            pValue->append(*pResolvedText);
        }
        else
        {
            pValue->push_back(cByte);
        }

    } while (true);

    return false;
}

bool XMLEventReader::HandleRAttributeName(std::unique_ptr<QName>& pName)
{
    std::unique_ptr<std::string> pNamePrefix(nullptr);
    std::unique_ptr<std::string> pNameLocalPart(new std::string());

    char cByte(ConsumeRWhitespace());

    if (cByte == '\0')
    {
        throw new std::runtime_error("Attribute incomplete.");
    }
    else if (cByte == ':')
    {
        throw new std::runtime_error("Attribute name incomplete.");
    }

    do
    {
        if (std::isspace(cByte, m_aLocale) != 0)
        {
            cByte = ConsumeRWhitespace();

            if (cByte != '\0')
            {
                runget();
            }
            else
            {
                throw new std::runtime_error("Attribute name incomplete.");
            }

            if (pNamePrefix == nullptr)
            {
                pNamePrefix.reset(new std::string);
            }
            else
            {
                // if (pNamePrefix->length() <= 0) will result in an empty prefix
                // of the QName anyway, but the reader could complain as well.

                std::reverse(pNamePrefix->begin(), pNamePrefix->end());

                if (pNamePrefix->length() > 0)
                {
                    // Other characters were already checked.
                    cByte = pNamePrefix->at(0);

                    if (std::isalnum(cByte, m_aLocale) != true &&
                        cByte != '_')
                    {
                        int nByte(cByte);
                        std::stringstream aMessage;
                        aMessage << "Character '" << cByte << "' (0x"
                                 << std::hex << std::uppercase << std::right << std::setfill('0') << std::setw(2) << nByte
                                 << ") not supported as first character of an attribute prefix name.";
                        throw new std::runtime_error(aMessage.str());
                    }
                }
            }

            cByte = pNameLocalPart->back();

            if (std::isalnum(cByte, m_aLocale) != true &&
                    cByte != '_')
            {
                throw new std::runtime_error("Attribute name malformed.");
            }

            std::reverse(pNameLocalPart->begin(), pNameLocalPart->end());

            pName = std::unique_ptr<QName>(new QName("", *pNameLocalPart, *pNamePrefix));

            return true;
        }
        else if (cByte == ':')
        {
            if (pNamePrefix != nullptr)
            {
                throw new std::runtime_error("There can't be two prefixes in attribute name.");
            }

            pNamePrefix.reset(new std::string());
        }
        else if (std::isalnum(cByte, m_aLocale) == true ||
                 cByte == '-' ||
                 cByte == '_' ||
                 cByte == '.')
        {
            if (pNamePrefix != nullptr)
            {
                pNamePrefix->push_back(cByte);
            }
            else
            {
                pNameLocalPart->push_back(cByte);
            }
        }
        else
        {
            int nByte(cByte);
            std::stringstream aMessage;
            aMessage << "Character '" << cByte << "' (0x"
                     << std::hex << std::uppercase << std::right << std::setfill('0') << std::setw(2) << nByte
                     << ") not supported in an attribute name.";
            throw new std::runtime_error(aMessage.str());
        }

        if (rget(cByte) != true)
        {
            throw new std::runtime_error("Attribute name incomplete.");
        }

    } while (true);

    return false;
}

void XMLEventReader::ResolveREntity(std::unique_ptr<std::string>& pResolvedText)
{
    // Should the code from ResolveREntity(std::unique_ptr<std::string>&)
    // be duplicated in order to avoid putting the '\0' char onto the
    // stack during runtime?
    return ResolveREntity('\0', pResolvedText);
}

/**
 * @param[in] cDelimiter Delimiter that will abort the attempt to read an entity name.
 */
void XMLEventReader::ResolveREntity(const char& cDelimiter, std::unique_ptr<std::string>& pResolvedText)
{
    if (pResolvedText != nullptr)
    {
        throw new std::invalid_argument("No nullptr passed.");
    }

    std::unique_ptr<std::string> pEntityName(new std::string);
    char cByte('\0');

    do
    {
        if (rget(cByte) != true)
        {
            pResolvedText = std::move(pEntityName);
            pResolvedText->insert(0, 1, ';');
            return;
        }

        if (cByte == '&')
        {
            break;
        }
        else if (cDelimiter != '\0' &&
                 cByte == cDelimiter)
        {
            runget();
            pResolvedText = std::move(pEntityName);
            pResolvedText->insert(0, 1, ';');
            return;
        }
        /** @todo Check for more illegal characters in entity names (whitespace?) */
        // '<' is illegal here, but not a breaking issue when reading
        // backwards/reverse, and as we're not checking well-formedness
        // yet, they're handled as normal characters for now.
        else if (cByte == '>' ||
                 cByte == ';')
        {
            runget();
            pResolvedText = std::move(pEntityName);
            pResolvedText->insert(0, 1, ';');
            return;
        }

        pEntityName->push_back(cByte);

    } while (true);

    if (pEntityName->length() <= 0)
    {
        throw new std::runtime_error("Entity has no name.");
    }

    std::map<std::string, std::string>::iterator iter = m_aREntityReplacementDictionary.find(*pEntityName);

    if (iter != m_aREntityReplacementDictionary.end())
    {
        pResolvedText = std::unique_ptr<std::string>(new std::string(iter->second));
    }
    else
    {
        std::reverse(pEntityName->begin(), pEntityName->end());

        std::stringstream aMessage;
        aMessage << "Unable to resolve entity '&" << *pEntityName << ";'.";
        throw new std::runtime_error(aMessage.str());
    }
}

/**
 * @retval Returns the first non-whitespace character or '\0' in
 *     case of begin-of-file.
 */
char XMLEventReader::ConsumeRWhitespace()
{
    char cByte('\0');

    do
    {
        if (rget(cByte) != true)
        {
            return '\0';
        }

        if (std::isspace(cByte, m_aLocale) == 0)
        {
            return cByte;
        }

    } while (true);
}

bool XMLEventReader::rget(char& c)
{
    if (m_aStream.eof() == true)
    {
        m_aStream.clear();
        m_aStream.seekg(0, std::ios_base::end);

        if (m_aStream.eof() == true)
        {
            throw new std::runtime_error("Stream operation failed.");
        }

        if (m_aStream.fail() == true)
        {
            throw new std::runtime_error("Stream operation failed.");
        }

        if (m_aStream.bad() == true)
        {
            throw new std::runtime_error("Stream is bad.");
        }
    }

    std::streampos nPosition(m_aStream.tellg());

    if (nPosition == 0)
    {
        return false;
    }

    m_aStream.seekg(-1, std::ios_base::cur);

    if (m_aStream.fail() == true)
    {
        throw new std::runtime_error("Stream operation failed.");
    }

    if (m_aStream.bad() == true)
    {
        throw new std::runtime_error("Stream is bad.");
    }

    m_aStream.get(c);

    if (m_aStream.fail() == true)
    {
        throw new std::runtime_error("Stream operation failed.");
    }

    if (m_aStream.bad() == true)
    {
        throw new std::runtime_error("Stream is bad.");
    }

    m_aStream.seekg(-1, std::ios_base::cur);

    if (m_aStream.fail() == true)
    {
        throw new std::runtime_error("Stream operation failed.");
    }

    if (m_aStream.bad() == true)
    {
        throw new std::runtime_error("Stream is bad.");
    }

    return true;
}

bool XMLEventReader::runget()
{
    if (m_aStream.eof() == true)
    {
        return false;
    }

    m_aStream.seekg(1, std::ios_base::cur);

    if (m_aStream.fail() == true)
    {
        throw new std::runtime_error("Stream operation failed.");
    }

    if (m_aStream.bad() == true)
    {
        throw new std::runtime_error("Stream is bad.");
    }

    // The mere m_aStream.seekg() won't set the EOF bit, only reading
    // attempts will do.

    return true;
}

}
