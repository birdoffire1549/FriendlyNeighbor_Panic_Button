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
            "#wrapper { background-color: #E6EFFF; padding: 20px; margin-left: auto; margin-right: auto; max-width: 700px; box-shadow: 3px 3px 3px #333 } "
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
        "<p>NOTE: Most of the settings below can only be changed by performing a factory reset."
        " To perform a factory reset hold the cancel button while cycling power, then continue to "
        "hold until device says the reset is complete.</p>"
        "<form name=\"settings\" method=\"post\" id=\"settings\" action=\"admin\"> "
            "<h2>WiFi</h2> "
            "SSID: ${ssid} <br> "
            "<h2>SMTP Settings</h2>"
            "Host: ${smtp_host} <br> "
            "Port: ${smtp_port} <br> "
            "User: ${smtp_user} <br> "
            "From Name: ${from_name} <br> "
            "From Email: ${from_email} <br> "
            "<h2>Application</h2> "
            "Owner: ${owner} <br> "
            "Message: ${message} <br> "
            "Panic Level: ${panic_level} <br> "
            "Semicolin Separated Recipients: <input maxlength=\"509\" type=\"text\" value=\"${recips}\" name=\"recips\" id=\"recips\"> <br> "
            "<h2>Admin</h2> "
            "Admin Password: <input maxlength=\"63\" type=\"password\" value=\"${admin_pwd}\" name=\"admin_pwd\" id=\"admin_pwd\"> <br> "
            "<br> "
            "<input type=\"submit\">"
        "</form>"
    };

    const char PROGMEM INITIAL_SETUP_PAGE[] = {
        "<form name=\"settings\" method=\"post\" id=\"settings\" action=\"admin\"> "
            "<h2>WiFi</h2> "
            "SSID: <input maxlength=\"32\" type=\"text\" value=\"${ssid}\" name=\"ssid\" id=\"ssid\"> <br> "
            "Password: <input maxlength=\"63\" type=\"text\" value=\"${pwd}\" name=\"pwd\" id=\"pwd\"> <br> "
            "<h2>SMTP Settings</h2>"
            "Host: <input maxlength=\"120\" type=\"text\" value=\"${smtp_host}\" name=\"smtp_host\" id=\"smtp_host\"> <br> "
            "Port: <input type=\"number\" value=\"${smtp_port}\" name=\"smtp_port\" id=\"smtp_port\"> <br> "
            "User: <input maxlength=\"120\" type=\"text\" value=\"${smtp_user}\" name=\"smtp_user\" id=\"smtp_user\"> <br> "
            "Password: <input maxlength=\"120\" type=\"password\" value=\"${smtp_pwd}\" name=\"smtp_pwd\" id=\"smtp_pwd\"> <br> "
            "From Name: <input maxlength=\"50\" type=\"text\" value=\"${from_name}\" name=\"from_name\" id=\"from_name\"> <br> "
            "From Email: <input maxlength=\"120\" type=\"text\" value=\"${from_email}\" name=\"from_email\" id=\"from_email\"> <br> "
            "<h2>Application</h2> "
            "Owner: <input maxlength=\"100\" type=\"text\" value=\"${owner}\" name=\"owner\" id=\"owner\"> <br> "
            "Message: <input maxlength=\"100\" type=\"text\" value=\"${message}\" name=\"message\" id=\"message\"> <br> "
            "Panic Level: <input type=\"number\" value=\"${panic_level}\" name=\"panic_level\" id=\"panic_level\"> <br> "
            "Semicolin Separated Recipients: <input maxlength=\"509\" type=\"text\" value=\"${recips}\" name=\"recips\" id=\"recips\"> <br> "
            "<h2>Admin</h2> "
            "Admin Password: <input maxlength=\"63\" type=\"password\" value=\"${admin_pwd}\" name=\"admin_pwd\" id=\"admin_pwd\"> <br> "
            "<br> "
            "<input type=\"submit\">"
        "</form>"
    };

    const char PROGMEM ROOT_PAGE[] = {
        "Device:\tFriendlyNeighbor Panic Button<br>"
        "Firmware Version:\t${firmware_version}<br>"
    };

#endif