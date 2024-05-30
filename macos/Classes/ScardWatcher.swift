import CryptoTokenKit

protocol ScardWatcherDelegate {
    func scardDidInsert(readerName: String)
    func scardDidRemove(readerName: String)
}

class ScardWatcher {
    private static let instance = ScardWatcher()
    static var shared: ScardWatcher {
        get { self.instance }
    }

    private let tokenWatcher: TKTokenWatcher
    private var _smartCards: [String : [TKTokenWatcher.TokenInfo]] = [:]
    var delegate: ScardWatcherDelegate?

    var smartCards: [String : Any] {
        get {
            var scards: [String : Any] = [:]
            for readerName in self._smartCards.keys {
                guard let tokenList = getTokenInfo(readerName: readerName) else {
                    continue
                }
                scards[readerName] = tokenList
            }
            return scards
        }
    }

    init() {
        self.tokenWatcher = TKTokenWatcher()
        self.tokenWatcher.setInsertionHandler() { tokenId in
            guard let tokenInfo = self.tokenWatcher.tokenInfo(forTokenID: tokenId),
                  let readerName = tokenInfo.slotName else {
                return
            }
            if self._smartCards.keys.contains(readerName) {
                self._smartCards[readerName]!.removeAll(where: { ($0.tokenID == tokenInfo.tokenID) })
                self._smartCards[readerName]!.append(tokenInfo)
            } else {
                self._smartCards[readerName] = [tokenInfo]
                self.tokenWatcher.addRemovalHandler(self.removalHandler, forTokenID: tokenId)
                DispatchQueue.main.async {
                    self.delegate?.scardDidInsert(readerName: readerName)
                }
            }
        }
    }

    private func removalHandler(tokenId: String) {
        guard let index = self._smartCards.firstIndex(where: { $1.contains(where: { $0.tokenID == tokenId }) }) else {
            return
        }
        let readerName = self._smartCards.keys[index]
        self._smartCards.remove(at: index)
        DispatchQueue.main.async {
            self.delegate?.scardDidRemove(readerName: readerName)
        }
    }

    func getTokenInfo(readerName: String) -> [[String : Any]]? {
        guard let tokens = self._smartCards[readerName], tokens.count > 0 else {
            return nil
        }
        var tokenList: [[String : Any]] = []
        for token in tokens {
            tokenList.append(token.toDictionary())
        }
        return tokenList
    }
}

extension TKTokenWatcher.TokenInfo {
    func toDictionary() -> [String : Any] {
        var dict: [String : Any] = ["tokenID" : self.tokenID]
        if let slotName = self.slotName {
            dict["slotName"] = slotName
        }
        if let driverName = self.driverName {
            dict["driverName"] = driverName
        }
        return dict
    }
}
