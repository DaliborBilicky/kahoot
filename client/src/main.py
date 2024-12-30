import customtkinter as ctk

ctk.set_appearance_mode("System")
ctk.set_default_color_theme("blue")

class QuizApp(ctk.CTk):
    def __init__(self):
        super().__init__()
        self.title("Kahoot")
        self.geometry("600x400")

        ctk.CTkLabel(self, text="Vitajte!", font=("Arial", 24)).pack(pady=20)
        ctk.CTkButton(self, text="Vytvoriť lobby", command=self.create_lobby).pack(pady=10)
        ctk.CTkButton(self, text="Pripojiť sa do lobby", command=self.join_lobby).pack(pady=10)
        ctk.CTkButton(self, text="Opustiť", command=self.destroy).pack(pady=10)

    def create_lobby(self):
        print("Vytváranie lobby...")

    def join_lobby(self):
        print("Pripájanie do lobby...")

if __name__ == "__main__":
    app = QuizApp()
    app.mainloop()
