
if __name__ == "__main__":
    import sys
    import json
    import numpy as np

    firstline = sys.stdin.readline()
    obj = json.loads(firstline)

    Np = obj['num_points']
    dt = obj['dt']
    L  = obj['L']
    Nt = obj['num_steps']
    Nint = obj['step_chunk']
    k = obj['k']
    d = obj['d']
    gifname = obj['gifname']

    numframes = int(Nt) // int(Nint) + 1
    maxinterv = 100
    maxinterv = min(maxinterv,numframes -1)
    accum = np.zeros((maxinterv,1))
    denom = np.zeros((maxinterv,1))
    for i in range(numframes):
        try:
            line = sys.stdin.readline()
            obj = json.loads(line)
            X = np.array(obj['X'])
        except:
            break
        center = np.mean(X,axis=1)
        X = X - center.reshape((3,1)) * np.ones((1,X.shape[1]))
        if not i:
            X0 = np.ndarray((maxinterv,X.shape[0],X.shape[1]))
            for j in range(maxinterv):
                X0[j,:,:] = X[:,:]
            continue
        for interv in range(1,maxinterv+1):
            if i % interv:
                continue
            r = X[:,:] - X0[interv-1,:,:]
            s_pro = r[0,:]*r[0,:] + r[1,:]*r[1,:] + r[2,:]*r[2,:]
            accum[interv-1] = accum[interv-1] + np.mean(s_pro)
            denom[interv-1] = denom[interv-1] + 1
            X0[interv-1,:,:] = X[:,:]

    out = accum / denom
    x = np.linspace(dt*Nint,dt*Nint*maxinterv,maxinterv)
    p = np.polyfit(x,out,1)
    print(f'Diffusion constant: {p[0] / 6.}')
