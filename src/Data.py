import concurrent.futures

class Utils:
    def __init__(self):
        pass
    
    def get_data_type(self,data_str: str) -> str:
        if data_str.startswith('"') and data_str.endswith('"'):
            return 'str'
        elif '.' in data_str:
            try:
                float(data_str)
                return 'float'
            except ValueError:
                pass
        try:
            int(data_str)
            return 'int'
        except ValueError:
            pass
        if data_str.lower() == 'true' or data_str.lower() == 'false':
            return 'bool'
        return 'str'


class Analytics(Utils):
    def __init__(self, data: dict[str:list]):
        super().__init__()
        self.data = data

    @property
    def datatypes(self) -> dict[str:list]:
        columns = self.data['columns']
        rows = self.data['rows']
        data = {_:{'types':set(),'detail':{},'rows_types':{}} for _ in columns}

        def process_row(row_idx):
            types = []
            for j in range(len(rows[row_idx])):
                dd = self.get_data_type(rows[row_idx][j])
                data[columns[j]]['types'].add(dd)
                if dd not in data[columns[j]]['rows_types']:
                    data[columns[j]]['rows_types'][dd]={'count':0,'rows':[],}

                data[columns[j]]['rows_types'][dd]['count'] += 1
                data[columns[j]]['rows_types'][dd]['rows'].append(row_idx)
                types.append(dd)

            for k in range(len(rows[row_idx])):
                for _ in data[columns[k]]['types']:
                    dd = self.get_data_type(rows[row_idx][k])
                    data[columns[k]]['rows_types'][_]['percent'] = data[columns[k]]['rows_types'][_]['count'] / len(rows)

        with concurrent.futures.ThreadPoolExecutor() as executor:
            executor.map(process_row, range(len(rows)))

        return data


class Data(Analytics):
    filepath: str

    def __init__(self, path: str):
        self.filepath = path
        self.filedata = open(path,'r').readlines()
        super().__init__(self.table)

    @property
    def columns(self) -> list[str]:
        return list(map(lambda x: self.clear_arr(x), self.filedata[0].split(',')))

    @property
    def rows(self) -> list[list]:
        return [list(map(lambda x: self.clear_arr(x), self.filedata[_+1].split(','))) for _ in range(len(self.filedata[1:]))]

    @property 
    def table(self) -> dict[str:list]:
        return {'columns': self.columns, "rows": self.rows}

    def clear_arr(self, x: str) -> str:
        return x.strip('\n').strip("'").strip('"')
