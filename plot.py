import pandas as pd
import matplotlib.pyplot as plt


df_add = pd.read_csv("add.csv")
df_add_mem = pd.read_csv("add_mem.csv")

df_delete = pd.read_csv("delete.csv")
df_delete_mem = pd.read_csv("delete_mem.csv")

df_select = pd.read_csv("selection.csv")
df_select_mem = pd.read_csv("selection_mem.csv")

df_update = pd.read_csv("update.csv")
df_update_mem = pd.read_csv("update_memory.csv")

plt.plot(df_add["index"], df_add["time"], color="green")
plt.title("Create Node")
plt.xlabel("Index")
plt.ylabel("Time")
plt.grid(True)
plt.show()

plt.plot(df_add_mem["index"], df_add_mem["memory"], color="green")
plt.title("Create Node")
plt.xlabel("Index")
plt.ylabel("Memory")
plt.grid(True)
plt.show()


plt.plot(df_delete["index"], df_delete["time"], color="green")
plt.title("Delete Node")
plt.xlabel("Index")
plt.ylabel("Time")
plt.grid(True)
plt.show()

plt.plot(df_delete_mem["index"], df_delete_mem["memory"], color="green")
plt.title("Delete Node")
plt.xlabel("Index")
plt.ylabel("Memory")
plt.grid(True)
plt.show()



# plt.plot(df_select["index"], df_select["time"], color="green")
# plt.title("Select Node")
# plt.xlabel("Index")
# plt.ylabel("Time")
# plt.grid(True)
# plt.show()

# plt.plot(df_select_mem["index"], df_select_mem["memory"], color="green")
# plt.title("Select Node")
# plt.xlabel("Index")
# plt.ylabel("Memory")
# plt.grid(True)
# plt.show()


# plt.plot(df_update["index"], df_update["time"], color="green")
# plt.title("Update Node")
# plt.xlabel("Index")
# plt.ylabel("Time")
# plt.grid(True)
# plt.show()

# plt.plot(df_update_mem["index"], df_update_mem["memory"], color="green")
# plt.title("Update Node")
# plt.xlabel("Index")
# plt.ylabel("Memory")
# plt.grid(True)
# plt.show()