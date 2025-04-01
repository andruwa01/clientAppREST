#ifndef HELPDEFINES_H
#define HELPDEFINES_H

// Constants
#define DATE_FORMAT "yyyy-MM-dd"

// Enable / disable defines

//#define TEST_INTERFACE_ONLY // enable/disable

#if defined TEST_INTERFACE_ONLY // TEST INTERFACE WITHOUT API
    #define TEST_JSON_INPUT
#else // TEST INTERFACE WITH API
    #define USE_API
#endif

// Printing defines
#define PRINT_DEBUG_JSON(jsonObject) \
    qDebug() << "\n<==============================>"; \
    qDebug() << "BEGIN JSON"; \
    qDebug() << jsonObject; \
    qDebug() << "END JSON"; \
    qDebug() << "<==============================>\n";

#endif // HELPDEFINES_H
