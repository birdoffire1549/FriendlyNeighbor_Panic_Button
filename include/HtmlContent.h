#ifndef HtmlContent_h
    #define HtmlContent_h

    #include <WString.h>
    #include <pgmspace.h>

    const char PROGMEM HTML_PAGE_TEMPLATE[] = {
        "<!DOCTYPE HTML> "
        "<html lang=\"en\"> "
        "<head> "
            "<title>${title}</title> "
            "<style> "
            "body { background-color: #FFFFFF; color: #000000; } "
            "h1 { text-align: center; background-color: #5878B0; color: #FFFFFF; border: 3px; border-radius: 15px; } "
            "h2 { text-align: center; background-color: #58ADB0; color: #FFFFFF; border: 3px; } "
            "#wrapper { background-color: #E6EFFF; padding: 20px; margin-left: auto; margin-right: auto; max-width: 750px; box-shadow: 3px 3px 3px #333 } "
            "#info { font-size: 30px; font-weight: bold; line-height: 150%; } "
            "</style> "
        "</head> "
        ""
        "<div id=\"wrapper\"> "
            "<h1>${heading}</h1> "
            "<div id=\"info\">${content}</div> "
        "</div> "
        "</html>"
    };

    const char PROGMEM ADMIN_PAGE[] = {
        "<form name=\"settings\" method=\"POST\" id=\"settings\" action=\"update\"> "
            "<h2>Configuration</h2> "
            "Settings as JSON: <textarea name=\"data\" rows=\"25\" cols=\"90\">${settings}</textarea>"
            "<br> "
            "<input type=\"submit\">"
        "</form>"
    };

    const char PROGMEM ROOT_PAGE[] = {
        "Device:\tFriendlyNeighbor Panic Button<br>"
        "Firmware Version:\t${firmware_version}<br>"
    };

#endif