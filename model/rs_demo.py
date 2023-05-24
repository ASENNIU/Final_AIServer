import torch

class RS_Model(torch.nn.Module):
    def __init__(self, item_num, embedding_size):
        super(RS_Model, self).__init__()

        # 定义一个简单的模型来拟合协同过滤，item_num即代表物品的个数
        self.layer = torch.nn.Sequential(
            torch.nn.Linear(embedding_size, item_num),
            torch.nn.Softmax(dim = 1)
        )

        for m in self.modules():
            if isinstance(m, torch.nn.Linear):
                torch.nn.init.xavier_normal_(m.weight, gain=1)

    def forward(self, input):
        output = self.layer(input)
        return output

if __name__ == "__main__":
    print("exec script...")
    model = RS_Model(100, 10)
    # model = torch.jit.load("RS_CO_MODE.pt")
    input = torch.rand(3, 10)
    model.eval()
    out = model(input)
    print(out)
    script_model = torch.jit.script(model)
    script_model.save("RS_CO_MODEL.pt")

