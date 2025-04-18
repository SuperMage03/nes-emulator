CXX = /usr/bin/clang++
CXXFLAGS = -std=c++20 -g
SRCDIR = src
BUILDDIR = build
TARGET := $(shell basename $(CURDIR))

SRCEXT = cpp
SOURCES = $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS = $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
DEPENDS = ${OBJECTS:.o=.d}
INC = -I include
LIB = -L lib
LINKEROPTIONS = -Wl,-rpath ./lib
SFMLLIB = -l sfml-system -l sfml-window -l sfml-graphics -l sfml-audio -l sfml-network

.PHONY: clean

release: $(TARGET)

debug: CXXFLAGS += -DDEBUG
debug: $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(LIB) $(SFMLLIB) $(LINKEROPTIONS) $^ -o $(TARGET)

$(BUILDDIR)/%.o : $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(INC) -MMD -c -o $@ $<

-include ${DEPENDS}

clean:
	rm -rf $(RM) -r ${DEPENDS} $(BUILDDIR) $(TARGET)
