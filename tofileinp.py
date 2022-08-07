import numpy as np
f = open("input_params.txt", "w")

# size of matrix is n and partition = k
n=1000
k=100
adj=np.random.randint(0,2,(n,n))

for i in range(0, n):
    for j in range(i, n):
        adj[i][j]=adj[j][i]

for i in range(0,n):
    adj[i][i] = 0
# print(adj)

f.write(str(k)+" "+str(n)+"\n")
# print(str(k)+" "+str(n))
f.write("  ")
# print(" ",end =" ")
for i in range(0,n-1):
    # print(i+1, end =" ")
    f.write(str(i+1)+" ")
# print(n)
f.write(str(n)+"\n")

for i in range(0,n):
    # print(i+1, end =" ")
    f.write(str(i + 1) + " ")
    for j in range(0, n-1):
        # print(adj[i][j], end =" ")
        f.write(str(adj[i][j]) + " ")
    # print(adj[i][n-1])
    f.write(str(adj[i][n-1]) + "\n")
f.close()