import tkinter as tk
from tkinter import ttk, messagebox, filedialog, scrolledtext
import subprocess, threading, os, sys, datetime

class CarteiroGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Carteiro GPU")
        self.root.geometry("750x600")
        self.root.configure(bg="#1e1e2e")
        self.targets = []
        self.running = False
        self.process = None
        if getattr(sys, 'frozen', False):
            base = sys._MEIPASS
        else:
            base = os.path.abspath(".")
        self.solver_exe = os.path.join(base, "puzzle_gpu.exe")
        self.kernel_file = os.path.join(base, "kernel.cl")
        self.build_ui()

    def build_ui(self):
        tk.Label(self.root, text="🔑 Carteiro GPU", fg="#3fb950", bg="#1e1e2e", font=("Segoe UI", 18, "bold")).pack(pady=15)
        # ... (resto da interface – exatamente igual ao script anterior)
        # Por brevidade, vou omitir o código repetitivo da UI aqui, mas você já tem o app.py completo do script anterior.
        # Use o app.py do script anterior, que está completo.

if __name__ == "__main__":
    root = tk.Tk()
    app = CarteiroGUI(root)
    root.mainloop()
