with open("0_to_3000_accurate","r") as f:
    accurate = [int(i) for i in f.read().split("\n")]


with open("comp.txt","r") as f:
    comp = [i for i in f.read().split("\n")]


for i in range(len(accurate)):
    correct = " ".join([str(int(i)) for i in [bool(accurate[i]>=5),bool(accurate[i]>=10),bool(accurate[i]>=26)]])
    if correct == comp[i]:
        print("WOW")
    else:
        print("OOBA")
        quit()