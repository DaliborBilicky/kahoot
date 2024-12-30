import customtkinter as ctk


def main():
    ctk.set_appearance_mode("System")
    ctk.set_default_color_theme("blue")

    app = ctk.CTk()
    app.title("CustomTkinter Window Example")
    app.geometry("400x300")

    app.mainloop()


if __name__ == "__main__":
    main()
