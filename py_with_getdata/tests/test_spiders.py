import csv
import os

from bs4 import BeautifulSoup
from config import web_config

# 假设 html_content 包含了爬取到的 HTML 数据
html_content = """
<ul>
 <strong>
  Minimum:
 </strong>
 <br/>
 <ul class="bb_ul">
  <li>
   <strong>
    OS *:
   </strong>
   Windows® 7 (32/64-bit)/Vista/XP
   <br/>
  </li>
  <li>
   <strong>
    Processor:
   </strong>
   1.7 GHz Processor or better
   <br/>
  </li>
  <li>
   <strong>
    Memory:
   </strong>
   512 MB RAM
   <br/>
  </li>
  <li>
   <strong>
    DirectX:
   </strong>
   Version 8.1
   <br/>
  </li>
  <li>
   <strong>
    Network:
   </strong>
   Broadband Internet connection
   <br/>
  </li>
  <li>
   <strong>
    Storage:
   </strong>
   15 GB available space
  </li>
 </ul>
</ul>
<ul class="bb_ul">
 <li>
  <strong>
   OS *:
  </strong>
  Windows® 7 (32/64-bit)/Vista/XP
  <br/>
 </li>
 <li>
  <strong>
   Processor:
  </strong>
  1.7 GHz Processor or better
  <br/>
 </li>
 <li>
  <strong>
   Memory:
  </strong>
  512 MB RAM
  <br/>
 </li>
 <li>
  <strong>
   DirectX:
  </strong>
  Version 8.1
  <br/>
 </li>
 <li>
  <strong>
   Network:
  </strong>
  Broadband Internet connection
  <br/>
 </li>
 <li>
  <strong>
   Storage:
  </strong>
  15 GB available space
 </li>
</ul>
<ul>
 <strong>
  Recommended:
 </strong>
 <br/>
 <ul class="bb_ul">
  <li>
   <strong>
    OS *:
   </strong>
   Windows® 7 (32/64-bit)
   <br/>
  </li>
  <li>
   <strong>
    Processor:
   </strong>
   Pentium 4 processor (3.0GHz, or better)
   <br/>
  </li>
  <li>
   <strong>
    Memory:
   </strong>
   1 GB RAM
   <br/>
  </li>
  <li>
   <strong>
    DirectX:
   </strong>
   Version 9.0c
   <br/>
  </li>
  <li>
   <strong>
    Network:
   </strong>
   Broadband Internet connection
   <br/>
  </li>
  <li>
   <strong>
    Storage:
   </strong>
   15 GB available space
  </li>
 </ul>
</ul>
<ul class="bb_ul">
 <li>
  <strong>
   OS *:
  </strong>
  Windows® 7 (32/64-bit)
  <br/>
 </li>
 <li>
  <strong>
   Processor:
  </strong>
  Pentium 4 processor (3.0GHz, or better)
  <br/>
 </li>
 <li>
  <strong>
   Memory:
  </strong>
  1 GB RAM
  <br/>
 </li>
 <li>
  <strong>
   DirectX:
  </strong>
  Version 9.0c
  <br/>
 </li>
 <li>
  <strong>
   Network:
  </strong>
  Broadband Internet connection
  <br/>
 </li>
 <li>
  <strong>
   Storage:
  </strong>
  15 GB available space
 </li>
</ul>
<ul>
 <strong>
  Minimum:
 </strong>
 <br/>
 <ul class="bb_ul">
  <li>
   <strong>
    OS:
   </strong>
   OS X version Leopard 10.5.8 and above
   <br/>
  </li>
  <li>
   <strong>
    Processor:
   </strong>
   1.7 GHz Processor or better
   <br/>
  </li>
  <li>
   <strong>
    Memory:
   </strong>
   1 GB RAM
   <br/>
  </li>
  <li>
   <strong>
    Graphics:
   </strong>
   NVIDIA GeForce 8 or higher, ATI X1600 or higher, Intel HD 3000 or higher
   <br/>
  </li>
  <li>
   <strong>
    Network:
   </strong>
   Broadband Internet connection
   <br/>
  </li>
  <li>
   <strong>
    Storage:
   </strong>
   15 GB available space
  </li>
 </ul>
</ul>
<ul class="bb_ul">
 <li>
  <strong>
   OS:
  </strong>
  OS X version Leopard 10.5.8 and above
  <br/>
 </li>
 <li>
  <strong>
   Processor:
  </strong>
  1.7 GHz Processor or better
  <br/>
 </li>
 <li>
  <strong>
   Memory:
  </strong>
  1 GB RAM
  <br/>
 </li>
 <li>
  <strong>
   Graphics:
  </strong>
  NVIDIA GeForce 8 or higher, ATI X1600 or higher, Intel HD 3000 or higher
  <br/>
 </li>
 <li>
  <strong>
   Network:
  </strong>
  Broadband Internet connection
  <br/>
 </li>
 <li>
  <strong>
   Storage:
  </strong>
  15 GB available space
 </li>
</ul>
<ul>
 <strong>
  Minimum:
 </strong>
 <br/>
 <ul class="bb_ul">
  <li>
   <strong>
    OS:
   </strong>
   Ubuntu 12.04
   <br/>
  </li>
  <li>
   <strong>
    Processor:
   </strong>
   Dual core from Intel or AMD at 2.8 GHz
   <br/>
  </li>
  <li>
   <strong>
    Memory:
   </strong>
   1 GB RAM
   <br/>
  </li>
  <li>
   <strong>
    Graphics:
   </strong>
   nVidia GeForce 8600/9600GT, ATI/AMD Radeon HD2600/3600 (Graphic Drivers: nVidia 310, AMD 12.11), OpenGL 2.1
   <br/>
  </li>
  <li>
   <strong>
    Network:
   </strong>
   Broadband Internet connection
   <br/>
  </li>
  <li>
   <strong>
    Storage:
   </strong>
   15 GB available space
   <br/>
  </li>
  <li>
   <strong>
    Sound Card:
   </strong>
   OpenAL Compatible Sound Card
  </li>
 </ul>
</ul>
<ul class="bb_ul">
 <li>
  <strong>
   OS:
  </strong>
  Ubuntu 12.04
  <br/>
 </li>
 <li>
  <strong>
   Processor:
  </strong>
  Dual core from Intel or AMD at 2.8 GHz
  <br/>
 </li>
 <li>
  <strong>
   Memory:
  </strong>
  1 GB RAM
  <br/>
 </li>
 <li>
  <strong>
   Graphics:
  </strong>
  nVidia GeForce 8600/9600GT, ATI/AMD Radeon HD2600/3600 (Graphic Drivers: nVidia 310, AMD 12.11), OpenGL 2.1
  <br/>
 </li>
 <li>
  <strong>
   Network:
  </strong>
  Broadband Internet connection
  <br/>
 </li>
 <li>
  <strong>
   Storage:
  </strong>
  15 GB available space
  <br/>
 </li>
 <li>
  <strong>
   Sound Card:
  </strong>
  OpenAL Compatible Sound Card
 </li>
</ul>

"""

# 使用 BeautifulSoup 解析 HTML
soup = BeautifulSoup(html_content, 'html.parser')

# 查找所有的 <ul> 元素
ul_elements = soup.find_all('ul')

# 创建一个列表，用于存储 CSV 行
csv_rows = []
for ul_element in ul_elements:
    headers = [strong.text.strip() for strong in ul_element.find_all('strong')]
    csv_rows.append(headers)
    # 获取数据
    data = [li.text.strip().replace('\n', '').replace('\t', '').replace(" ", "") for li in ul_element.find_all('li')]
    csv_rows.append(data)

# 使用 csv 模块将数据转换成 CSV 格式
csv_data = "\n".join([",".join(row) for row in csv_rows])

# 现在，你可以将 csv_data 存储在数据库字段中
# 例如，使用 SQL 语句插入或更新数据库记录
# INSERT INTO your_table (csv_column) VALUES ('csv_data');
# 或者，更新已有记录的 csv_column 字段
# UPDATE your_table SET csv_column = 'csv_data' WHERE your_condition;

print(os.getcwd())

with open(r'../data/output/result.csv', 'w', newline='', encoding='utf-8') as csvfile:
    csvwriter = csv.writer(csvfile)

    # 使用 csv.writer 写入数据
    csvwriter.writerows(csv.reader(csv_data.splitlines()))

print('CSV 文件已生成：output.csv')
