#ifndef HELPDEFINES_H
#define HELPDEFINES_H

// Constants

#define COLUMN_TITLE       0
#define COLUMN_DESCRIPTION 1
#define COLUMN_DUE_DATE    2
#define COLUMN_STATUS 	   3
#define COLUMN_EMPLOYEE	   4

#define DATE_FORMAT "yyyy-MM-dd"
#define DISPLAY_COLUMNS 5

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
