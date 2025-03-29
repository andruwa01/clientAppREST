#ifndef HELPDEFINES_H
#define HELPDEFINES_H

// Constants

#define DATE_FORMAT "yyyy-MM-dd"
#define DISPLAY_COLUMNS 4

// Enable / disable dfines

#define TEST_JSON_INPUT

// Printing defines

#define PRINT_DEBUG_JSON(jsonObject) \
    qDebug() << "\n<==============================>"; \
    qDebug() << "BEGIN JSON"; \
    qDebug() << jsonObject; \
    qDebug() << "END JSON"; \
    qDebug() << "<==============================>\n";

#endif // HELPDEFINES_H
