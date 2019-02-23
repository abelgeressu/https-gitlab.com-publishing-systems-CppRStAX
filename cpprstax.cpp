/* Copyright (C) 2017-2019 Stephan Kreutzer
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
 * @file $/cpprstax.cpp
 * @brief Demo program.
 * @author Stephan Kreutzer
 * @since 2018-11-11
 */

#include "XMLInputFactory.h"
#include <memory>
#include <iostream>
#include <fstream>

typedef std::unique_ptr<cpprstax::XMLEventReader> XMLEventReader;
typedef std::unique_ptr<cpprstax::XMLEvent> XMLEvent;

int Run(std::istream& aStream);



int main(int argc, char* argv[])
{
    std::cout << "CppRStAX Copyright (C) 2017-2019 Stephan Kreutzer\n"
              << "This program comes with ABSOLUTELY NO WARRANTY.\n"
              << "This is free software, and you are welcome to redistribute it\n"
              << "under certain conditions. See the GNU Affero General Public License 3\n"
              << "or any later version for details. Also, see the source code repository\n"
              << "https://gitlab.com/publishing-systems/CppRStAX/ and\n"
              << "the project website http://www.publishing-systems.org.\n"
              << std::endl;

    std::unique_ptr<std::ifstream> pStream = nullptr;

    try
    {
        if (argc >= 2)
        {
            pStream = std::unique_ptr<std::ifstream>(new std::ifstream);
            pStream->open(argv[1]);

            if (pStream->is_open() != true)
            {
                std::cout << "Couldn't open input file '" << argv[1] << "'.";
                return -1;
            }

            Run(*pStream);

            pStream->close();
        }
        else
        {
            // Run(std::cin) doesn't work because the buffered input stream
            // is a temporary source that can only be read once.
            std::cout << "Usage:\n\n\tcpprstax <input-xml-file>\n" << std::endl;
            return 1;
        }
    }
    catch (std::exception* pException)
    {
        std::cout << "Exception: " << pException->what() << std::endl;

        if (pStream != nullptr)
        {
            if (pStream->is_open() == true)
            {
                pStream->close();
            }
        }

        return -1;
    }

    return 0;
}

int Run(std::istream& aStream)
{
    cpprstax::XMLInputFactory aFactory;
    XMLEventReader pReader = aFactory.createXMLEventReader(aStream);

    // Instead of looking at XMLEvents sequentially, one could
    // also implement a "parse tree" to react to XMLEvents, so
    // writing state machines can be avoided because of the
    // implicit call tree context, pretty much like CppRStAX
    // does it's parsing.

    // This just moves the stream to the end of the input. Usually, the
    // backwards/reverse interface is there to move back and forwards when
    // the reader is positioned somewhere in the middle of the stream/elements,
    // to navigate the structure without a need to keep track in memory of what
    // was encountered earlier, but there are other use cases like if aStream
    // is already moved to the end (aStream.seekg(0, std::ios_base::end)),
    // or the not supported yet case of a stream that provides the characters
    // in reverse order while moving "forward".
    while (pReader->hasNext() == true)
    {
        pReader->nextEvent();
    }

    while (pReader->hasPrevious() == true)
    {
        XMLEvent pEvent = pReader->previousEvent();

        if (pEvent->isStartElement() == true)
        {
            cpprstax::StartElement& aStartElement = pEvent->asStartElement();
            const cpprstax::QName& aName = aStartElement.getName();

            std::string strTag("<");
            std::string strPrefix(aName.getPrefix());

            if (!strPrefix.empty())
            {
                strTag += strPrefix;
                strTag += ":";
            }

            strTag += aName.getLocalPart();

            for (std::list<std::shared_ptr<cpprstax::Attribute>>::iterator iter = aStartElement.getAttributes()->begin();
                 iter != aStartElement.getAttributes()->end();
                 iter++)
            {
                const cpprstax::QName& aAttributeName = (*iter)->getName();

                strTag += " ";

                std::string strAttributePrefix(aAttributeName.getPrefix());

                if (!strAttributePrefix.empty())
                {
                    strTag += strAttributePrefix;
                    strTag += ":";
                }

                strTag += aAttributeName.getLocalPart();
                strTag += "=\"";

                const std::string& strCharacters((*iter)->getValue());

                for (std::string::const_iterator iter = strCharacters.begin();
                    iter != strCharacters.end();
                    iter++)
                {
                    switch (*iter)
                    {
                    case '\"':
                        strTag += "&quot;";
                        break;
                    case '&':
                        strTag += "&amp;";
                        break;
                    case '<':
                        strTag += "&lt;";
                        break;
                    case '>':
                        strTag += "&gt;";
                        break;
                    default:
                        strTag += *iter;
                        break;
                    }
                }

                strTag += "\"";
            }

            strTag += ">";

            std::cout << strTag;
        }
        else if (pEvent->isEndElement() == true)
        {
            cpprstax::EndElement& aEndElement = pEvent->asEndElement();
            cpprstax::QName aName = aEndElement.getName();

            std::string strTag("</");
            std::string strPrefix(aName.getPrefix());

            if (!strPrefix.empty())
            {
                strTag += strPrefix;
                strTag += ":";
            }

            strTag += aName.getLocalPart();
            strTag += ">";

            std::cout << strTag;
        }
        else if (pEvent->isCharacters() == true)
        {
            cpprstax::Characters& aCharacters = pEvent->asCharacters();
            const std::string& strCharacters(aCharacters.getData());

            for (std::string::const_iterator iter = strCharacters.begin();
                 iter != strCharacters.end();
                 iter++)
            {
                switch (*iter)
                {
                case '&':
                    std::cout << "&amp;";
                    break;
                case '<':
                    std::cout << "&lt;";
                    break;
                case '>':
                    std::cout << "&gt;";
                    break;
                default:
                    std::cout << *iter;
                    break;
                }
            }
        }
        else if (pEvent->isComment() == true)
        {
            std::cout << "<!--" << pEvent->asComment().getText() << "-->";
        }
        else if (pEvent->isProcessingInstruction() == true)
        {
            const cpprstax::ProcessingInstruction& aPI = pEvent->asProcessingInstruction();

            std::cout << "<?" << aPI.getTarget() << " " << aPI.getData() << "?>";
        }
        else
        {

        }
    }

    return 0;
}
