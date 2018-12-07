# Copyright (C) 2017-2018  Stephan Kreutzer
#
# This file is part of CppRStAX.
#
# CppRStAX is free software: you can redistribute it and/or modify it under
# the terms of the GNU Affero General Public License version 3 or any later
# version of the license, as published by the Free Software Foundation.
#
# CppRStAX is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Affero General Public License 3 for more details.
#
# You should have received a copy of the GNU Affero General Public License 3
# along with CppRStAX. If not, see <http://www.gnu.org/licenses/>.



.PHONY: build clean



CFLAGS = -std=c++11 -Wall -Werror -Wextra -pedantic



build: cpprstax



cpprstax: cpprstax.cpp XMLInputFactory.o XMLEventReader.o XMLEvent.o QName.o Attribute.o StartElement.o EndElement.o Characters.o ProcessingInstruction.o Comment.o
	g++ cpprstax.cpp QName.o Attribute.o StartElement.o EndElement.o Characters.o Comment.o ProcessingInstruction.o XMLEvent.o XMLEventReader.o XMLInputFactory.o -o cpprstax $(CFLAGS)

XMLInputFactory.o: XMLInputFactory.h XMLInputFactory.cpp
	g++ XMLInputFactory.cpp -c $(CFLAGS)

XMLEventReader.o: XMLEventReader.h XMLEventReader.cpp
	g++ XMLEventReader.cpp -c $(CFLAGS)

XMLEvent.o: XMLEvent.h XMLEvent.cpp
	g++ XMLEvent.cpp -c $(CFLAGS)

StartElement.o: StartElement.h StartElement.cpp
	g++ StartElement.cpp -c $(CFLAGS)

Attribute.o: Attribute.h Attribute.cpp
	g++ Attribute.cpp -c $(CFLAGS)

EndElement.o: EndElement.h EndElement.cpp
	g++ EndElement.cpp -c $(CFLAGS)

Characters.o: Characters.h Characters.cpp
	g++ Characters.cpp -c $(CFLAGS)	

ProcessingInstruction.o: ProcessingInstruction.h ProcessingInstruction.cpp
	g++ ProcessingInstruction.cpp -c $(CFLAGS)

Comment.o: Comment.h Comment.cpp
	g++ Comment.cpp -c $(CFLAGS)	

QName.o: QName.h QName.cpp
	g++ QName.cpp -c $(CFLAGS)

clean:
	rm -f ./cpprstax
	rm -f ./cpprstax.o
	rm -f ./XMLInputFactory.o
	rm -f ./XMLEventReader.o
	rm -f ./XMLEvent.o
	rm -f ./Attribute.o
	rm -f ./StartElement.o
	rm -f ./EndElement.o
	rm -f ./Characters.o
	rm -f ./ProcessingInstruction.o
	rm -f ./Comment.o
	rm -f ./QName.o
