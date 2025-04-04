### Должно быть предустановлено:
- qmake6 (6.2.4) `sudo apt install qmake6`.
- qt библиотеки.

Собираем и запускаем клиентское приложение:
1. `$ mkdir build`: создаём папку для файлов сборки.
2. `$ cd build`: переходим в папку где будем собирать проект.
3. `$ qmake6 ../clientApp/clientApp.pro`: генерируем make файлы.
4. `$ make`.
5. `$ ./clientApp`: запускаем приложение (перед этим должно быть запущено серверное приложение)

### Описание функционала
- [x] Добавление / удаление сотрудников и задач (в том числе вложенных задач) - через верхнее меню. Перед использованием нужно выбрать задачу или сотрудника. 
- [x] Изменение ячеек сотрудников / задач (для редактирования - двойной щелчок мышью), в том числе:
	- [x] Изменение даты с помощью виджета, дата показывается красным, если дата меньше текущей.
	- [x] Возможность назначить сотрудника для выполнения задачи через выпадающий список.
- [x] Синхронизация с базой данных через REST API сервер при старте программы, при изменении ячеек таблицы.

### Возможные улучшения
- [ ] void ApiClient::updateTask() -> убрать поле id, вместо этого извлекать поле из task (task.id).
- [ ] bool TreeItemModel::insertRows() -> return true; после m_apiClient->createTask(task) - это не совсем корректно сделано, можно изменить.
- [ ] исправить ситуацию, при который слоты из treeitemmodel вызываются вручную, это неправильно строго говоря.
- [ ] убрать в void MainWindow::handleTaskStatuschange ui->treeView->viewport()->update(); ---> всё обновляется и без этого.
- [ ] в void TreeItemModel::onEmployeeRemoved(int employeeId)
	- [ ] убрать / добавить обработку флага `updated`, потому что пока что он не спользуется.
	- [ ] убрать createIndex() ?