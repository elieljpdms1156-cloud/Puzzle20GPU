import tkinter as tk
from tkinter import ttk, messagebox, filedialog, scrolledtext
import subprocess
import threading
import os
import sys
import datetime

class CarteiroGPU:
    def __init__(self, root):
        self.root = root
        self.root.title("Carteiro GPU - Bitcoin Puzzle Solver")
        self.root.geometry("750x600")
        self.root.configure(bg="#1e1e2e")
        self.root.resizable(False, False)
        
        self.targets = []
        self.running = False
        self.process = None
        
        if getattr(sys, 'frozen', False):
            base_path = sys._MEIPASS
        else:
            base_path = os.path.abspath(".")
        self.solver_exe = os.path.join(base_path, "puzzle_gpu.exe")
        self.kernel_file = os.path.join(base_path, "kernel.cl")
        
        self.setup_ui()
        
    def setup_ui(self):
        title = tk.Label(self.root, text="🔑 Carteiro GPU", 
                        fg="#3fb950", bg="#1e1e2e", 
                        font=("Segoe UI", 18, "bold"))
        title.pack(pady=15)
        
        subtitle = tk.Label(self.root, text="Colecione múltiplas carteiras e busque com GPU",
                           fg="#8b949e", bg="#1e1e2e", font=("Segoe UI", 10))
        subtitle.pack(pady=(0,15))
        
        frame_alvos = tk.LabelFrame(self.root, text=" Carteiras Alvo (hash160) ", 
                                    fg="#c9d1d9", bg="#161b22", 
                                    font=("Segoe UI", 10, "bold"),
                                    padx=10, pady=10)
        frame_alvos.pack(fill="x", padx=20, pady=5)
        
        self.listbox = tk.Listbox(frame_alvos, bg="#0d1117", fg="#c9d1d9", 
                                  height=6, selectbackground="#30363d",
                                  font=("Consolas", 9))
        self.listbox.pack(fill="x", pady=5)
        
        btn_frame = tk.Frame(frame_alvos, bg="#161b22")
        btn_frame.pack(fill="x")
        
        self.entry_hash = tk.Entry(btn_frame, bg="#0d1117", fg="#c9d1d9", 
                                   insertbackground="white", font=("Consolas", 10))
        self.entry_hash.pack(side="left", fill="x", expand=True, padx=(0,5))
        
        tk.Button(btn_frame, text="Adicionar", command=self.add_target,
                 bg="#238636", fg="white", font=("Segoe UI", 9, "bold"),
                 relief="flat", padx=10).pack(side="left", padx=2)
        tk.Button(btn_frame, text="Arquivo", command=self.load_file,
                 bg="#21262d", fg="#c9d1d9", font=("Segoe UI", 9),
                 relief="flat", padx=10).pack(side="left", padx=2)
        tk.Button(btn_frame, text="Limpar", command=self.clear_targets,
                 bg="#21262d", fg="#c9d1d9", font=("Segoe UI", 9),
                 relief="flat", padx=10).pack(side="left", padx=2)
        
        frame_config = tk.LabelFrame(self.root, text=" Configuração da Busca ",
                                     fg="#c9d1d9", bg="#161b22",
                                     font=("Segoe UI", 10, "bold"),
                                     padx=10, pady=10)
        frame_config.pack(fill="x", padx=20, pady=5)
        
        row1 = tk.Frame(frame_config, bg="#161b22")
        row1.pack(fill="x", pady=5)
        tk.Label(row1, text="Início (hex):", fg="#8b949e", bg="#161b22", 
                font=("Segoe UI", 9)).pack(side="left")
        self.entry_start = tk.Entry(row1, bg="#0d1117", fg="#c9d1d9", width=20,
                                    font=("Consolas", 10))
        self.entry_start.pack(side="left", padx=5)
        self.entry_start.insert(0, "80000")
        
        tk.Label(row1, text="Fim (hex):", fg="#8b949e", bg="#161b22",
                font=("Segoe UI", 9)).pack(side="left", padx=(15,0))
        self.entry_end = tk.Entry(row1, bg="#0d1117", fg="#c9d1d9", width=20,
                                  font=("Consolas", 10))
        self.entry_end.pack(side="left", padx=5)
        self.entry_end.insert(0, "FFFFF")
        
        row2 = tk.Frame(frame_config, bg="#161b22")
        row2.pack(fill="x", pady=5)
        tk.Label(row2, text="Threads:", fg="#8b949e", bg="#161b22",
                font=("Segoe UI", 9)).pack(side="left")
        self.entry_threads = tk.Entry(row2, bg="#0d1117", fg="#c9d1d9", width=5,
                                      font=("Consolas", 10))
        self.entry_threads.pack(side="left", padx=5)
        self.entry_threads.insert(0, "2")
        
        ctrl_frame = tk.Frame(self.root, bg="#1e1e2e")
        ctrl_frame.pack(pady=15)
        
        self.btn_start = tk.Button(ctrl_frame, text="▶ INICIAR BUSCA", command=self.start_search,
                                   bg="#238636", fg="white", font=("Segoe UI", 11, "bold"),
                                   relief="flat", width=18, height=2)
        self.btn_start.pack(side="left", padx=5)
        
        self.btn_stop = tk.Button(ctrl_frame, text="⏹ PARAR", command=self.stop_search,
                                  bg="#da3633", fg="white", font=("Segoe UI", 11, "bold"),
                                  relief="flat", width=18, height=2, state="disabled")
        self.btn_stop.pack(side="left", padx=5)
        
        self.progress = ttk.Progressbar(self.root, length=650, mode='determinate')
        self.progress.pack(pady=10)
        
        self.label_status = tk.Label(self.root, text="Pronto para buscar", 
                                     fg="#8b949e", bg="#1e1e2e", font=("Segoe UI", 9))
        self.label_status.pack()
        
        self.text_log = scrolledtext.ScrolledText(self.root, bg="#0d1117", fg="#c9d1d9",
                                                   height=10, font=("Consolas", 9),
                                                   wrap=tk.WORD)
        self.text_log.pack(fill="both", expand=True, padx=20, pady=15)
        
    def log(self, msg):
        self.text_log.insert("end", f"[{datetime.datetime.now().strftime('%H:%M:%S')}] {msg}\n")
        self.text_log.see("end")
        
    def add_target(self):
        h = self.entry_hash.get().strip()
        if len(h) == 40 and all(c in "0123456789abcdefABCDEF" for c in h):
            if h not in self.targets:
                self.targets.append(h)
                self.listbox.insert("end", h)
                self.entry_hash.delete(0, "end")
                self.log(f"✅ Adicionado: {h[:10]}...")
        else:
            messagebox.showerror("Erro", "Hash160 inválido (40 caracteres hex)")
            
    def load_file(self):
        path = filedialog.askopenfilename(filetypes=[("Arquivos de texto", "*.txt")])
        if path:
            with open(path) as f:
                added = 0
                for line in f:
                    h = line.strip()
                    if len(h) == 40 and h not in self.targets:
                        self.targets.append(h)
                        self.listbox.insert("end", h)
                        added += 1
                self.log(f"📂 Carregados {added} hash160s do arquivo.")
                
    def clear_targets(self):
        self.targets.clear()
        self.listbox.delete(0, "end")
        self.log("🗑️ Lista de alvos limpa.")
        
    def start_search(self):
        if not self.targets:
            messagebox.showwarning("Aviso", "Adicione pelo menos um hash160.")
            return
        if self.running:
            return
        
        try:
            lo = int(self.entry_start.get(), 16)
            hi = int(self.entry_end.get(), 16)
            threads = int(self.entry_threads.get())
            if lo >= hi or threads < 1:
                raise ValueError
        except:
            messagebox.showerror("Erro", "Valores inválidos para intervalo ou threads.")
            return
        
        with open("alvos.txt", "w") as f:
            for t in self.targets:
                f.write(t + "\n")
        
        self.running = True
        self.btn_start["state"] = "disabled"
        self.btn_stop["state"] = "normal"
        self.progress["value"] = 0
        self.log(f"🚀 Iniciando busca: {hex(lo)} -> {hex(hi)} com {threads} threads")
        self.log(f"🎯 {len(self.targets)} alvos configurados")
        
        threading.Thread(target=self.run_solver, args=(lo, hi, threads), daemon=True).start()
        
    def run_solver(self, lo, hi, threads):
        try:
            cmd = [self.solver_exe, hex(lo)[2:], hex(hi)[2:], str(threads)]
            self.process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                           text=True, bufsize=1, universal_newlines=True)
            for line in self.process.stdout:
                self.root.after(0, self.log, line.strip())
                if "ENCONTRADO" in line:
                    self.root.after(0, self.search_done)
                    return
            self.process.wait()
            self.root.after(0, self.search_done)
        except Exception as e:
            self.root.after(0, self.log, f"❌ Erro: {e}")
            self.root.after(0, self.search_done)
            
    def stop_search(self):
        if self.process:
            self.process.terminate()
        self.running = False
        self.btn_start["state"] = "normal"
        self.btn_stop["state"] = "disabled"
        self.log("⏹ Busca interrompida.")
        
    def search_done(self):
        self.running = False
        self.btn_start["state"] = "normal"
        self.btn_stop["state"] = "disabled"
        self.log("✅ Busca finalizada.")

if __name__ == "__main__":
    root = tk.Tk()
    app = CarteiroGPU(root)
    root.mainloop()
