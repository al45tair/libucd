from bintrees import FastRBTree

class RangeSet (object):
    def __init__(self, ranges):
        self.tree = FastRBTree()

        for r in ranges:
            self.add(r)
        
    def add(self, rng):
        rs,re = rng
        ds,de = (None, None)
        try:
            ls,le = self.tree.floor_item(rs)
            # If we get here, ls <= rng.start
            if le >= rs - 1:
                de = ds = ls
                rs = ls
        except KeyError:
            pass

        for s,e in self.tree[rs:re + 2].items():
            if ds is None:
                ds = s
            de = s
            if e > re:
                re = e

        if ds is not None:
            del self.tree[ds:de + 1]
        self.tree[rs] = re
        
    def remove(self, rng):
        rs,re = rng
        ds,de = (rs, re)
        try:
            ls,le = self.tree.floor_item(rs)

            # Truncate an initial range, if any
            if ls < rs and le >= rs:
                self.tree[ls] = rs - 1
                if le > re:
                    self.tree[re + 1] = le
        except KeyError:
            pass

        ins=None
        for s,e in self.tree[rs:re + 1].items():
            de = s
            if e > re:
                self.tree[re + 1] = e

        del self.tree[ds:de + 1]

    def range_containing(self, pos):
        ls,le = self.tree.floor_item(pos)
        if ls <= pos and le >= pos:
            return (ls, le)
        return None
    
    def __repr__(self):
        return 'RangeSet([%s])' % ', '.join(['(%s, %s)' % (s,e) for s,e in self.tree.items()])

    def __iter__(self):
        return iter(self.tree)
    
    def items(self):
        return self.tree.items()

