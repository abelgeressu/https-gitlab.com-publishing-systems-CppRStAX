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
 * @file $/XMLEventReader.h
 * @author Stephan Kreutzer
 * @since 2018-11-11
 */

#ifndef _CPPRSTAX_XMLEVENTREADER_H
#define _CPPRSTAX_XMLEVENTREADER_H

#include "XMLEvent.h"
#include "Attribute.h"
#include <istream>
#include <locale>
#include <memory>
#include <queue>
#include <map>

namespace cpprstax
{

class XMLEventReader
{
public:
    XMLEventReader(std::istream& aStream);
    ~XMLEventReader();

    bool hasNext();
    std::unique_ptr<XMLEvent> nextEvent();

    bool hasPrevious();
    std::unique_ptr<XMLEvent> previousEvent();

public:
    int addToEntityReplacementDictionary(const std::string& strName, const std::string& strReplacementText);

protected:
    // Forward direction.
    bool HandleTag();
    bool HandleTagStart(const char& cFirstByte);
    bool HandleTagEnd();
    bool HandleText(const char& cFirstByte);
    bool HandleProcessingInstruction();
    bool HandleProcessingInstructionTarget(std::unique_ptr<std::string>& pTarget);
    bool HandleMarkupDeclaration();
    bool HandleComment();
    bool HandleAttributes(const char& cFirstByte, std::unique_ptr<std::list<std::unique_ptr<Attribute>>>& pAttributes);
    bool HandleAttributeName(const char& cFirstByte, std::unique_ptr<QName>& pName);
    bool HandleAttributeValue(std::unique_ptr<std::string>& pValue);
    void ResolveEntity(std::unique_ptr<std::string>& pResolvedText);
    char ConsumeWhitespace();

protected:
    // Backward direction.
    bool HandleRTag();
    bool HandleRTagStartEnd(const char& cFirstByte);
    bool HandleRTagStart(char cFirstByte, std::unique_ptr<StartElement>& pStartElement);
    bool HandleRTagName(const char& cFirstByte, std::unique_ptr<QName>& pName);
    bool HandleRText(const char& cFirstByte);
    bool HandleRProcessingInstruction();
    bool HandleRComment();
    bool HandleRAttributes(const char& cFirstByte, std::unique_ptr<std::list<std::unique_ptr<Attribute>>>& pAttributes);
    bool HandleRAttributeValue(const char& cDelimiter, std::unique_ptr<std::string>& pValue);
    bool HandleRAttributeName(std::unique_ptr<QName>& pName);
    void ResolveREntity(std::unique_ptr<std::string>& pResolvedText);
    void ResolveREntity(const char& cDelimiter, std::unique_ptr<std::string>& pResolvedText);
    char ConsumeRWhitespace();

    bool rget(char& c);
    bool runget();

protected:
    std::istream& m_aStream;
    std::locale m_aLocale;
    bool m_bHasNextCalled;
    bool m_bHasPreviousCalled;
    std::queue<std::unique_ptr<XMLEvent>> m_aEvents;
    bool m_bEventsAreForwardDirection;
    std::map<std::string, std::string> m_aEntityReplacementDictionary;
    std::map<std::string, std::string> m_aREntityReplacementDictionary;

};

}

#endif
