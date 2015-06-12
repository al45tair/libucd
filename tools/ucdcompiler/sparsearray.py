from bintrees import FastRBTree

class SparseArray (object):
    def __init__(self):
        self.tree = FastRBTree()

    def __len__(self):
        try:
            k,v = self.tree.max_item()
        except KeyError:
            return 0
        return k + len(v)
    
    def __getitem__(self, ndx):
        try:
            base, chunk = self.tree.floor_item(ndx)
        except KeyError:
            return None
        offset = ndx - base
        if offset < len(chunk):
            return chunk[offset]
        else:
            return None

    def __setitem__(self, ndx, item):
        try:
            base, chunk = self.tree.floor_item(ndx)
        except KeyError:
            try:
                base, chunk = self.tree.ceiling_item(ndx)
            except KeyError:
                self.tree[ndx] = [item]
                return
            if ndx + 1 == base:
                chunk.insert(0,item)
                del self.tree[base]
                self.tree[ndx] = chunk
                return

        if base > ndx:
            self.tree[ndx] = [item]
            return

        offset = ndx - base
        if offset < len(chunk):
            chunk[offset] = item
        else:
            nextbase, nextchunk = (None, None)
            try:
                nextbase, nextchunk = self.tree.succ_item(base)
            except KeyError:
                pass

            if offset == len(chunk):
                chunk.append(item)
                if offset + 1 == nextbase:
                    chunk += nextchunk
                    del self.tree[nextbase]
            elif offset + 1 == nextbase:
                nextchunk.insert(0,item)
                del self.tree[nextbase]
                self.tree[ndx] = nextchunk
            else:
                self.tree[ndx] = [item]

    def __delitem__(self, ndx):
        base, chunk = self.tree.floor_item(ndx)
        offset = ndx - base
        if offset < len(chunk):
            before = chunk[:offset]
            after = chunk[offset + 1:]
            if len(before):
                self.tree[base] = before
            else:
                del self.tree[base]
            if len(after):
                self.tree[ndx + 1] = after

    def items(self):
        for k,vs in self.tree.items():
            for n,v in enumerate(vs):
                yield (k+n,v)

    def runs(self):
        return self.tree.items()

    def run_count(self):
        return len(self.tree)
    
    def __repr__(self):
        arep = []
        for k,v in self.tree.items():
            arep.append('[%r]=%s' % (k, ', '.join([repr(item) for item in v])))
        return 'SparseArray(%s)' % ', '.join(arep)
