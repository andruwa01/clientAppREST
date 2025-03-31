#ifndef HELPDEFINES_H
#define HELPDEFINES_H

// Constants
#define DATE_FORMAT "yyyy-MM-dd"

// Enable / disable defines
//#define TEST_JSON_INPUT
#define USE_API  // Uncomment to enable API integration

// Printing defines
#define PRINT_DEBUG_JSON(jsonObject) \
    qDebug() << "\n<==============================>"; \
    qDebug() << "BEGIN JSON"; \
    qDebug() << jsonObject; \
    qDebug() << "END JSON"; \
    qDebug() << "<==============================>\n";

#endif // HELPDEFINES_H
