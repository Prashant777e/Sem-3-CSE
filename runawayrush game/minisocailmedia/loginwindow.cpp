#include "loginwindow.h"
#include "ui_loginwindow.h"
#include "database.h"
#include <QMessageBox>
#include <QInputDialog>

LoginWindow::LoginWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoginWindow)
{
    ui->setupUi(this);
    setWindowTitle("Social Chat - Login");
    setFixedSize(520, 380);
}

LoginWindow::~LoginWindow() {
    delete ui;
}

void LoginWindow::on_loginButton_clicked() {
    QString username = ui->usernameEdit->text().trimmed();
    QString password = ui->passwordEdit->text().trimmed();

    if (username.isEmpty() || password.isEmpty()) {
        ui->statusLabel->setText("Please fill all fields!");
        return;
    }

    User* user = Database::instance().authenticateUser(username, password);

    if (user) {
        ui->statusLabel->setStyleSheet("color: green;");
        ui->statusLabel->setText("Login successful!");

        emit loginSuccessful(user->getUserId());
        delete user;
        close();
    } else {
        ui->statusLabel->setStyleSheet("color: red;");
        ui->statusLabel->setText("Invalid credentials!");
    }
}

void LoginWindow::on_registerButton_clicked() {
    bool ok;
    QString username = ui->usernameEdit->text().trimmed();
    QString password = ui->passwordEdit->text().trimmed();
    QString fullName = QInputDialog::getText(this, "Register",
                                             "Enter your full name:",
                                             QLineEdit::Normal, "", &ok);

    if (!ok || fullName.isEmpty()) return;

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter username and password!");
        return;
    }

    if (Database::instance().registerUser(username, password, fullName)) {
        QMessageBox::information(this, "Success","Account created! You can now login.");
        ui->statusLabel->setStyleSheet("color: green;");
        ui->statusLabel->setText("Registration successful!");
    } else {
        QMessageBox::warning(this, "Error","Username already exists!");
    }
}
