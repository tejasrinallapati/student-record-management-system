#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STUD_FILE "students.dat"
#define CSV_FILE  "students.csv"
#define MAX_STUDENTS 2000
#define MAX_SUBJECTS 3   

/* ======================== STRUCTS ======================== */

typedef struct {
    char id[20];
    char name[50];
    char branch[20];
    char section[10];
    float cgpa;
    char phone[15];
    char password[20];

    /* Attendance & Marks */
    float attendance;               
    float marks[MAX_SUBJECTS];      
    float percent;                  
    char  grade[3];                 
} Student;

/* ===================== INPUT HELPERS ===================== */

void readLine(char *buf, size_t size) {
    if (fgets(buf, (int)size, stdin)) {
        buf[strcspn(buf, "\n")] = '\0';
    } else if (size > 0) buf[0] = '\0';
}

int readInt() {
    char line[64];
    int x;
    while (1) {
        fgets(line, sizeof(line), stdin);
        if (sscanf(line, "%d", &x) == 1) return x;
        printf("Enter valid number: ");
    }
}

float readFloat() {
    char line[64];
    float x;
    while (1) {
        fgets(line, sizeof(line), stdin);
        if (sscanf(line, "%f", &x) == 1) return x;
        printf("Enter valid float: ");
    }
}

void pauseScreen() {
    printf("\nPress ENTER to continue...");
    char dummy[8];
    readLine(dummy, sizeof(dummy));
}

/* ================== PROGRESS / GRADE ================= */

void computeProgress(Student *s) {
    float total = 0;
    for (int i = 0; i < MAX_SUBJECTS; i++)
        total += s->marks[i];

    s->percent = total / MAX_SUBJECTS;

    if      (s->percent >= 90) strcpy(s->grade, "O");
    else if (s->percent >= 80) strcpy(s->grade, "A");
    else if (s->percent >= 70) strcpy(s->grade, "B");
    else if (s->percent >= 60) strcpy(s->grade, "C");
    else if (s->percent >= 50) strcpy(s->grade, "D");
    else strcpy(s->grade, "F");
}

/* ================== FILE HELPERS ================= */

int loadAllStudents(Student arr[], int max) {
    FILE *fp = fopen(STUD_FILE, "rb");
    if (!fp) return 0;

    int n = 0;
    while (n < max && fread(&arr[n], sizeof(Student), 1, fp) == 1) n++;

    fclose(fp);
    return n;
}

void saveAllStudents(Student arr[], int count) {
    FILE *fp = fopen(STUD_FILE, "wb");
    if (!fp) return;
    fwrite(arr, sizeof(Student), count, fp);
    fclose(fp);
}

int getStudentById(const char *id, Student *out) {
    FILE *fp = fopen(STUD_FILE, "rb");
    if (!fp) return 0;

    Student s;
    while (fread(&s, sizeof(Student), 1, fp) == 1) {
        if (strcmp(s.id, id) == 0) {
            if (out) *out = s;
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

/* ====================== CSV EXPORT ======================= */

void autoExportCSV() {
    Student arr[MAX_STUDENTS];
    int n = loadAllStudents(arr, MAX_STUDENTS);
    if (n == 0) return;

    FILE *fp = fopen(CSV_FILE, "w");
    if (!fp) return;

    fprintf(fp, "ID,Name,Branch,Section,CGPA,Phone,Attendance,C++,DAA,Coding Skills,Percent,Grade\n");

    for (int i = 0; i < n; i++) {
        fprintf(fp, "%s,%s,%s,%s,%.2f,%s,%.2f,%.2f,%.2f,%.2f,%.2f,%s\n",
            arr[i].id, arr[i].name, arr[i].branch, arr[i].section, arr[i].cgpa,
            arr[i].phone, arr[i].attendance,
            arr[i].marks[0], arr[i].marks[1], arr[i].marks[2],
            arr[i].percent, arr[i].grade
        );
    }
    fclose(fp);
}

/* =================== ADMIN CRUD =================== */

void adminAddStudent() {
    Student s;
    memset(&s, 0, sizeof(Student));

    FILE *fp = fopen(STUD_FILE, "ab");
    if (!fp) {
        printf("File error.\n");
        pauseScreen();
        return;
    }

    printf("\n----- ADD STUDENT -----\n");

    printf("ID: ");
    readLine(s.id, sizeof(s.id));

    Student tmp;
    if (getStudentById(s.id, &tmp)) {
        printf("ID already exists.\n");
        fclose(fp);
        pauseScreen();
        return;
    }

    printf("Name   : "); readLine(s.name, sizeof(s.name));
    printf("Branch : "); readLine(s.branch, sizeof(s.branch));
    printf("Section: "); readLine(s.section, sizeof(s.section));
    printf("CGPA   : "); s.cgpa = readFloat();
    printf("Phone  : "); readLine(s.phone, sizeof(s.phone));

    s.attendance = 0;
    for (int i = 0; i < MAX_SUBJECTS; i++) s.marks[i] = 0;
    computeProgress(&s);

    snprintf(s.password, sizeof(s.password), "%s@pass", s.id);

    fwrite(&s, sizeof(Student), 1, fp);
    fclose(fp);

    printf("Student added. Default password: %s\n", s.password);

    autoExportCSV();
    pauseScreen();
}

/* View all */
void adminViewAllStudents() {
    FILE *fp = fopen(STUD_FILE, "rb");
    if (!fp) {
        printf("No data.\n");
        pauseScreen();
        return;
    }

    Student s;
    printf("\n----- ALL STUDENTS -----\n");

    while (fread(&s, sizeof(Student), 1, fp) == 1) {
        printf("\nID: %s\nName: %s\nBranch: %s\nSection: %s\nCGPA: %.2f\nPhone: %s\n",
               s.id, s.name, s.branch, s.section, s.cgpa, s.phone);

        printf("Attendance: %.2f %%\n", s.attendance);
        printf("Marks: C++ %.1f | DAA %.1f | Coding %.1f\n",
               s.marks[0], s.marks[1], s.marks[2]);
        printf("Percent: %.2f | Grade: %s\n", s.percent, s.grade);
    }

    fclose(fp);
    pauseScreen();
}

/* Update */
void adminUpdateStudent() {
    char id[20];
    printf("\nEnter student ID to update: ");
    readLine(id, sizeof(id));

    FILE *fp = fopen(STUD_FILE, "rb+");
    if (!fp) {
        printf("No data.\n");
        pauseScreen();
        return;
    }

    Student s;
    int found = 0;

    while (fread(&s, sizeof(Student), 1, fp) == 1) {
        if (strcmp(s.id, id) == 0) {
            found = 1;

            printf("New Name   : "); readLine(s.name, sizeof(s.name));
            printf("New Branch : "); readLine(s.branch, sizeof(s.branch));
            printf("New Section: "); readLine(s.section, sizeof(s.section));
            printf("New CGPA   : "); s.cgpa = readFloat();
            printf("New Phone  : "); readLine(s.phone, sizeof(s.phone));

            computeProgress(&s);

            fseek(fp, -sizeof(Student), SEEK_CUR);
            fwrite(&s, sizeof(Student), 1, fp);
            break;
        }
    }

    fclose(fp);

    printf(found ? "Updated.\n" : "ID not found.\n");
    autoExportCSV();
    pauseScreen();
}

/* Delete */
void adminDeleteStudent() {
    char id[20];
    printf("Enter ID to delete: ");
    readLine(id, sizeof(id));

    FILE *fp = fopen(STUD_FILE, "rb");
    FILE *tmp = fopen("temp.dat", "wb");

    if (!fp || !tmp) {
        printf("File error.\n");
        pauseScreen();
        return;
    }

    Student s;
    int found = 0;

    while (fread(&s, sizeof(Student), 1, fp) == 1) {
        if (strcmp(s.id, id) == 0)
            found = 1;
        else
            fwrite(&s, sizeof(Student), 1, tmp);
    }

    fclose(fp);
    fclose(tmp);

    remove(STUD_FILE);
    rename("temp.dat", STUD_FILE);

    printf(found ? "Deleted.\n" : "ID not found.\n");
    autoExportCSV();
    pauseScreen();
}

/* =================== SEARCH =================== */

void searchStudents() {
    int ch;
    printf("\n1. Search by ID\n2. Search by Branch & Section\nChoose: ");
    ch = readInt();

    FILE *fp = fopen(STUD_FILE, "rb");
    if (!fp) {
        printf("No data.\n");
        pauseScreen();
        return;
    }

    Student s;
    int found = 0;

    if (ch == 1) {
        char id[20];
        printf("ID: "); readLine(id, sizeof(id));

        while (fread(&s, sizeof(Student), 1, fp) == 1) {
            if (strcmp(s.id, id) == 0) {
                found = 1;
                printf("\nID: %s\nName: %s\nBranch: %s\nSection: %s\nCGPA: %.2f\n",
                       s.id, s.name, s.branch, s.section, s.cgpa);

                printf("Attendance: %.2f\n", s.attendance);
                printf("Marks: %.1f %.1f %.1f\n",
                       s.marks[0], s.marks[1], s.marks[2]);
                printf("Percent: %.2f | Grade: %s\n", s.percent, s.grade);
            }
        }

    } else if (ch == 2) {
        char br[20], sec[10];

        printf("Branch : "); readLine(br, sizeof(br));
        printf("Section: "); readLine(sec, sizeof(sec));

        while (fread(&s, sizeof(Student), 1, fp) == 1) {
            if (strcmp(s.branch, br) == 0 && strcmp(s.section, sec) == 0) {
                found = 1;

                printf("\nID: %s | %s | CGPA %.2f | Attendance %.2f\n",
                       s.id, s.name, s.cgpa, s.attendance);

                printf("C++: %.1f | DAA: %.1f | Coding: %.1f\n",
                       s.marks[0], s.marks[1], s.marks[2]);
            }
        }
    }

    fclose(fp);
    if (!found) printf("No matching records.\n");

    pauseScreen();
}

/* ========== STAFF: UPDATE ATTENDANCE + MARKS ============= */

void staffUpdateAttendanceMarks() {
    char id[20];
    printf("\nEnter ID: ");
    readLine(id, sizeof(id));

    FILE *fp = fopen(STUD_FILE, "rb+");
    if (!fp) {
        printf("No data.\n");
        pauseScreen();
        return;
    }

    Student s;
    int found = 0;

    while (fread(&s, sizeof(Student), 1, fp) == 1) {
        if (strcmp(s.id, id) == 0) {
            found = 1;

            printf("New Attendance (0-100): ");
            s.attendance = readFloat();

            printf("Enter marks:\n");
            printf("C++          : "); s.marks[0] = readFloat();
            printf("DAA          : "); s.marks[1] = readFloat();
            printf("Coding Skills: "); s.marks[2] = readFloat();

            computeProgress(&s);

            fseek(fp, -sizeof(Student), SEEK_CUR);
            fwrite(&s, sizeof(Student), 1, fp);
            break;
        }
    }

    fclose(fp);

    printf(found ? "Updated.\n" : "ID not found.\n");
    autoExportCSV();
    pauseScreen();
}

/* ================= STUDENT SIDE ================= */

void studentViewMyDetails(const char sid[]) {
    Student s;
    if (!getStudentById(sid, &s)) {
        printf("Record not found.\n");
    } else {
        printf("\n----- MY DETAILS -----\n");
        printf("ID: %s\nName: %s\nBranch: %s\nSection: %s\nCGPA: %.2f\nPhone: %s\n",
               s.id, s.name, s.branch, s.section, s.cgpa, s.phone);
        printf("Attendance: %.2f %%\n", s.attendance);
    }
    pauseScreen();
}

void studentViewProgress(const char sid[]) {
    Student s;
    if (!getStudentById(sid, &s)) {
        printf("Record not found.\n");
    } else {
        printf("\n----- PROGRESS REPORT -----\n");
        printf("ID: %s | %s\nBranch: %s | Section: %s\n",
               s.id, s.name, s.branch, s.section);

        printf("\nMarks:\nC++: %.2f\nDAA: %.2f\nCoding Skills: %.2f\n",
               s.marks[0], s.marks[1], s.marks[2]);

        printf("\nPercent: %.2f\nGrade: %s\nAttendance: %.2f %%\n",
               s.percent, s.grade, s.attendance);
    }
    pauseScreen();
}

/* ================= LOGIN ================= */

int adminLogin() {
    char u[32], p[32];
    printf("\nUsername: "); readLine(u, sizeof(u));
    printf("Password: "); readLine(p, sizeof(p));

    return (strcmp(u, "admin") == 0 && strcmp(p, "admin123") == 0);
}

int staffLogin() {
    char u[32], p[32];
    printf("\nStaff Username: "); readLine(u, sizeof(u));
    printf("Password: "); readLine(p, sizeof(p));

    return (strcmp(u, "staff") == 0 && strcmp(p, "staff123") == 0);
}

int studentLogin(char outId[]) {
    char id[20], pass[20];
    Student s;

    printf("\nStudent ID: "); readLine(id, sizeof(id));
    printf("Password : "); readLine(pass, sizeof(pass));

    FILE *fp = fopen(STUD_FILE, "rb");
    if (!fp) return 0;

    while (fread(&s, sizeof(Student), 1, fp) == 1) {
        if (strcmp(s.id, id) == 0 && strcmp(s.password, pass) == 0) {
            fclose(fp);
            strcpy(outId, id);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

/* ================= MENUS ================= */

void adminMenu() {
    int c;
    while (1) {
        printf("\n=== ADMIN MENU ===\n");
        printf("1. Add Student\n");
        printf("2. View All Students\n");
        printf("3. Update Student\n");
        printf("4. Delete Student\n");
        printf("5. Search\n");
        printf("6. Export to CSV\n");
        printf("0. Logout\nChoose: ");
        c = readInt();

        if (c == 1) adminAddStudent();
        else if (c == 2) adminViewAllStudents();
        else if (c == 3) adminUpdateStudent();
        else if (c == 4) adminDeleteStudent();
        else if (c == 5) searchStudents();
        else if (c == 6) { autoExportCSV(); printf("Exported.\n"); pauseScreen(); }
        else if (c == 0) return;
        else printf("Invalid.\n");
    }
}

void staffMenu() {
    int c;
    while (1) {
        printf("\n=== STAFF MENU ===\n");
        printf("1. Search Students\n");
        printf("2. Update Attendance & Marks\n");
        printf("0. Logout\nChoose: ");
        c = readInt();

        if (c == 1) searchStudents();
        else if (c == 2) staffUpdateAttendanceMarks();
        else if (c == 0) return;
        else printf("Invalid.\n");
    }
}

void studentMenu(const char sid[]) {
    int c;
    while (1) {
        printf("\n=== STUDENT MENU ===\n");
        printf("1. My Details\n");
        printf("2. My Progress\n");
        printf("0. Logout\nChoose: ");
        c = readInt();

        if (c == 1) studentViewMyDetails(sid);
        else if (c == 2) studentViewProgress(sid);
        else if (c == 0) return;
        else printf("Invalid.\n");
    }
}

/* ================= MAIN ================= */

int main() {
    int ch;
    char sid[20];

    while (1) {
        printf("\n===== STUDENT RECORD SYSTEM =====\n");
        printf("1. Admin Login\n");
        printf("2. Staff Login\n");
        printf("3. Student Login\n");
        printf("0. Exit\nChoose: ");
        ch = readInt();

        if (ch == 1 && adminLogin()) adminMenu();
        else if (ch == 2 && staffLogin()) staffMenu();
        else if (ch == 3 && studentLogin(sid)) studentMenu(sid);
        else if (ch == 0) break;
        else printf("Invalid.\n");
    }

    return 0;
}